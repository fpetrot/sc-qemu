#include <glib.h>

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "qemu/main-loop.h"
#include "qemu/timer.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "sysemu/cpus.h"
#include "exec/gdbstub.h"
#include "qapi/error.h"
#include "qom/cpu.h"

#include "sc-qemu.h"
#include "qemu-context-priv.h"
#include "sc-object-priv.h"

qemu_context* SC_QEMU_INIT_SYM(sc_qemu_init_struct *s);

typedef struct mmio_ctx mmio_ctx;

struct mmio_ctx {
    qemu_context *qemu_ctx;
    uint32_t base;
};

typedef void (*ctor_fn)(void);
static GArray *ctor_fns = NULL;
static size_t ctor_count = 0;

static inline void lock_iothread(bool was_locked)
{
    if (!was_locked) {
        qemu_mutex_lock_iothread();
    }
}

static inline void unlock_iothread(bool was_locked)
{
    if (!was_locked) {
        qemu_mutex_unlock_iothread();
    }
}

static uint64_t sc_mmio_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    mmio_ctx *ctx = (mmio_ctx*) opaque;
    sc_qemu_io_attr attr;

    if (!ctx->qemu_ctx->elaboration_done) {
        /* Ignore reads while elaboration is ongoing */
        return 0;
    }

    attr.cpuid = current_cpu->cpu_index;

    return ctx->qemu_ctx->sysc.read(ctx->qemu_ctx->opaque,
                                    ctx->base + offset, size, &attr);
}

static void sc_mmio_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
    mmio_ctx *ctx = (mmio_ctx*) opaque;
    sc_qemu_io_attr attr;

    if (!ctx->qemu_ctx->elaboration_done) {
        /* Ignore writes while elaboration is ongoing */
        return;
    }

    attr.cpuid = current_cpu->cpu_index;

    ctx->qemu_ctx->sysc.write(ctx->qemu_ctx->opaque,
                              ctx->base + offset, value, size, &attr);
}

static const MemoryRegionOps sc_mmio_ops = {
    .read = sc_mmio_read,
    .write = sc_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


/* vl.c */
bool main_loop_should_exit(void);
/* ---- */

static MemoryRegion* map_io(qemu_context *ctx, MemoryRegion *root, uint64_t base, uint64_t size)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    MemoryRegion *mmio = g_new(MemoryRegion, 1);
    mmio_ctx *m_ctx = g_new(mmio_ctx, 1);

    m_ctx->base = base;
    m_ctx->qemu_ctx = ctx;

    memory_region_init_io(mmio, NULL, &sc_mmio_ops, m_ctx, "sc-mmio", size);

    memory_region_add_subregion(root, base, mmio);

    unlock_iothread(was_locked);

    return mmio;
}

static void sc_qemu_map_io(qemu_context *ctx, uint32_t base_address,
                           uint32_t size)
{
    MemoryRegion *mr = MEMORY_REGION(ctx->root_mr.obj);
    map_io(ctx, mr, base_address, size);
}

static void sc_qemu_map_dmi(qemu_context *ctx, uint32_t base_address,
                            uint32_t size, void* data, bool readonly)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    MemoryRegion *sysmem = get_system_memory();
    MemoryRegion *dmi = g_new(MemoryRegion, 1);
    static int dmi_count = 0;

    char name[] = "sc-dmi  ";
    snprintf(name, sizeof(name), "sc-dmi%d", dmi_count);
    dmi_count++;

    if (!readonly) {
        memory_region_init_ram_ptr(dmi, NULL, name, size, data);
    } else {
        mmio_ctx *m_ctx = g_new(mmio_ctx, 1);
        m_ctx->base = base_address;
        m_ctx->qemu_ctx = ctx;

        memory_region_init_rom_device_ptr(dmi, NULL, &sc_mmio_ops, m_ctx,
                                          name, size, data);
    }

    vmstate_register_ram_global(dmi);

    memory_region_add_subregion(sysmem, base_address, dmi);

    unlock_iothread(was_locked);
}

static void sc_qemu_start_gdbserver(qemu_context *ctx, const char *port)
{
    gdbserver_start(port);
    qemu_system_debug_request();
}

static qemu_context *main_thread_ctx = NULL;

static bool qemu_has_work(void)
{
    CPUState *cpu;

    if (qemu_clock_deadline_ns_all(QEMU_CLOCK_VIRTUAL) != -1) {
        return true;
    }

    CPU_FOREACH(cpu) {
        if (cpu_is_stopped(cpu)) {
            continue;
        }

        if (!cpu->halted || cpu_has_work(cpu)) {
            return true;
        }
    }

    return false;
}

static bool sc_qemu_cpu_loop(qemu_context *ctx, int64_t *elapsed, bool *has_work)
{
    int64_t before;

    main_thread_ctx = ctx;

    if (!qemu_mutex_iothread_locked()) {
        /* This is the first time we are called */
        ctx->elaboration_done = true;
        qemu_thread_get_self(first_cpu->thread);
        qemu_system_reset(false);
        qemu_mutex_lock_iothread();
    }

    ctx->main_status = MAIN_OK;

    if (elapsed) {
        before = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    }

    qemu_coroutine_enter(first_cpu->coroutine);

    if (elapsed) {
        *elapsed = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - before;
    }

    if (has_work) {
        *has_work = qemu_has_work();
    }

    return (ctx->main_status != MAIN_OK) || main_loop_should_exit();
}

static void call_qemu_ctors(void)
{
    size_t i;
    ctor_fn ctor;

    for (i = 0; i < ctor_count; i++) {
        ctor = g_array_index(ctor_fns, ctor_fn, i);
        ctor();
    }

    g_array_free(ctor_fns, TRUE);
    ctor_fns = NULL;
    ctor_count = 0;

}

static void init_q_import(qemu_import *q_import)
{
    /* SystemC to QEMU fonctions */
    q_import->cpu_loop = sc_qemu_cpu_loop;
    q_import->map_io = sc_qemu_map_io;
    q_import->map_dmi = sc_qemu_map_dmi;
    q_import->start_gdbserver = sc_qemu_start_gdbserver;
    q_import->object_new = sc_qemu_object_new;
    q_import->object_property_set_bool = sc_qemu_object_property_set_bool;
    q_import->object_property_set_int = sc_qemu_object_property_set_int;
    q_import->object_property_set_str = sc_qemu_object_property_set_str;
    q_import->object_property_set_link = sc_qemu_object_property_set_link;
    q_import->object_mmio_map = sc_qemu_object_mmio_map;
    q_import->object_gpio_connect = sc_qemu_object_gpio_connect;
    q_import->object_gpio_update = sc_qemu_object_gpio_update;
    q_import->object_gpio_register_cb = sc_qemu_object_gpio_register_cb;
    q_import->cpu_get_id = sc_qemu_cpu_get_id;
    q_import->object_get_root_mr = sc_qemu_object_get_root_mr;
}

static void setup_limiter_timer(sc_qemu_init_struct *s, qemu_context *ctx)
{
    const int64_t max_run_time = s->max_run_time ? s->max_run_time : SCALE_MS;

    /* Limit the time we can last into the main loop.
     * Default to 1ms if not specified. */
    qemu_tcg_set_kick_period(max_run_time);
}


int qemu_main(int argc, char const * argv[], char **envp);

static void * qemu_io_thread(void *arg)
{
    qemu_mutex_lock_iothread();
    do {
        main_loop_wait(false);
    } while(!main_loop_should_exit());

    return NULL;
}

qemu_context* SC_QEMU_INIT_SYM(sc_qemu_init_struct *s)
{
    char const * qemu_base_argv[] = {
        "",
        "-M", "none",
        "-m", "2048",
        "-monitor", "null",
        "-serial", "null",
        "-nographic",
        "-accel", "tcg,thread=single",
        /*"-d", "in_asm,cpu,exec",*/
        /*"-D", "qemu.log",*/
    };

    char icount_option[] = "shift=00,align=off";

    char const * qemu_argv[ARRAY_SIZE(qemu_base_argv) + 2] = { 0 };
    int qemu_argc = ARRAY_SIZE(qemu_base_argv);

    memcpy(qemu_argv, qemu_base_argv, sizeof(qemu_base_argv));

    if (s->max_run_time > 0) {
        /* Enable icount */
        snprintf(icount_option, sizeof(icount_option), "shift=%d,align=off", s->cpu_mips_shift);
        qemu_argv[qemu_argc++] = "-icount";
        qemu_argv[qemu_argc++] = icount_option;
    }

    /* QEMU initialization */
    call_qemu_ctors();
    qemu_main(qemu_argc, qemu_argv, NULL);

    init_q_import(s->q_import);

    qemu_context *ctx;
    ctx = g_malloc0(sizeof(qemu_context));
    ctx->opaque = s->opaque;
    memcpy(&(ctx->sysc), &(s->sc_import), sizeof(systemc_import));

    setup_limiter_timer(s, ctx);

    ctx->root_mr.ctx = ctx;
    if (s->map_whole_as) {
        ctx->root_mr.obj = OBJECT(map_io(ctx, get_system_memory(), 0, ((uint64_t)UINT32_MAX)+1));
    } else {
        ctx->root_mr.obj = OBJECT(get_system_memory());
    }

    QemuThread *main_thread = g_malloc0(sizeof(QemuThread));
    qemu_thread_create(main_thread, "io-thread", qemu_io_thread, NULL, QEMU_THREAD_JOINABLE);

    qemu_mutex_unlock_iothread();
    return ctx;
}

void sc_qemu_do_register_ctor(ctor_fn f)
{
    if (ctor_fns == NULL) {
        ctor_fns = g_array_new(FALSE, FALSE, sizeof (ctor_fn));
    }

    g_array_append_val(ctor_fns, f);
    ctor_count++;
}

void QEMU_NORETURN __real_exit(int status);
void QEMU_NORETURN __wrap_abort(void);
void QEMU_NORETURN __wrap_exit(int status);

static QEMU_NORETURN void main_thread_abort(enum ScQemuMainLoopStatus s)
{
    if (current_cpu && qemu_coroutine_self() == current_cpu->coroutine) {
        main_thread_ctx->main_status = s;
        qemu_coroutine_yield();
    }

    fprintf(stderr, "Internal QEMU error. Cannot continue\n");
    __real_exit(1);
}

void QEMU_NORETURN __wrap_abort(void)
{
    main_thread_abort(MAIN_ABORT);
}

void QEMU_NORETURN __wrap_exit(int status)
{
    if (!status) {
        main_thread_abort(MAIN_EXIT);
    } else {
        main_thread_abort(MAIN_ABORT);
    }

}

