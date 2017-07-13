#include <glib.h>

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "qemu/main-loop.h"
#include "qemu/timer.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "exec/gdbstub.h"
#include "qapi/error.h"

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
#ifdef SYNCHRONOUS_IO
    sc_qemu_io_attr attr;

    attr.cpuid = current_cpu->cpu_index;

    return ctx->qemu_ctx->sysc.read(ctx->qemu_ctx->opaque,
                                    ctx->base + offset, size, &attr);
#else
    qemu_context *qctx = ctx->qemu_ctx;

    qctx->main_status = MAIN_MMIO_READ;
    qctx->mmio_addr = ctx->base + offset;
    qctx->mmio_size = size;
    qctx->mmio_attr.cpuid = current_cpu->cpu_index;

    qemu_cpu_cond_wait(&qctx->io_cond);

    return qctx->mmio_value;
#endif

}

static void sc_mmio_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
    mmio_ctx *ctx = (mmio_ctx*) opaque;
#ifdef SYNCHRONOUS_IO
    sc_qemu_io_attr attr;

    attr.cpuid = current_cpu->cpu_index;

    ctx->qemu_ctx->sysc.write(ctx->qemu_ctx->opaque,
                              ctx->base + offset, value, size, &attr);
#else
    qemu_context *qctx = ctx->qemu_ctx;

    qctx->main_status = MAIN_MMIO_WRITE;
    qctx->mmio_addr = ctx->base + offset;
    qctx->mmio_value = value;
    qctx->mmio_size = size;
    qctx->mmio_attr.cpuid = current_cpu->cpu_index;

    qemu_cpu_cond_wait(&qctx->io_cond);
#endif
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
    map_io(ctx, ctx->root_mr, base_address, size);
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

static void deadline_cb(void *opaque)
{
    qemu_context *ctx = (qemu_context *) opaque;

    timer_mod(ctx->deadline, qemu_clock_get_ns(ctx->limiter_clock) + ctx->max_run_time_ns);
}


static qemu_context *main_thread_ctx = NULL;

static void do_one_main_loop(qemu_context *ctx)
{
    int64_t before;
    static int last_io = 0;

    main_thread_ctx = ctx;

    if (!qemu_mutex_iothread_locked()) {
        /* This is the first time we are called */
        qemu_mutex_lock_iothread();
        vm_start();
    }

    ctx->main_status = MAIN_OK;
    before = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);

    last_io = main_loop_wait(last_io > 0);

    ctx->last_elapsed = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - before;
}

static bool sc_qemu_cpu_loop(qemu_context *ctx, int64_t *elapsed)
{
    do {
        do_one_main_loop(ctx);

        switch (ctx->main_status) {
        case MAIN_MMIO_READ:
            ctx->mmio_value = ctx->sysc.read(ctx->opaque, ctx->mmio_addr,
                                             ctx->mmio_size, &ctx->mmio_attr);
            qemu_cond_signal(&ctx->io_cond);
            break;
        case MAIN_MMIO_WRITE:
            ctx->sysc.write(ctx->opaque, ctx->mmio_addr, ctx->mmio_value,
                            ctx->mmio_size, &ctx->mmio_attr);
            qemu_cond_signal(&ctx->io_cond);
            break;
        default:
            break;
        }
    } while (ctx->main_status > MAIN_ABORT);

    if (elapsed) {
        *elapsed = ctx->last_elapsed;
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
    q_import->object_mmio_map = sc_qemu_object_mmio_map;
    q_import->object_gpio_connect = sc_qemu_object_gpio_connect;
    q_import->object_gpio_update = sc_qemu_object_gpio_update;
    q_import->object_gpio_register_cb = sc_qemu_object_gpio_register_cb;
    q_import->cpu_get_id = sc_qemu_cpu_get_id;
}

static void setup_limiter_timer(sc_qemu_init_struct *s, qemu_context *ctx)
{
    if (s->max_run_time > 0) {
        /* Limit the number of instructions QEMU executes when calling the main loop */
        ctx->limiter_clock = QEMU_CLOCK_VIRTUAL;
        ctx->max_run_time_ns = s->max_run_time;
    } else {
        /* Limit the time we can last into the main loop */
        ctx->limiter_clock = QEMU_CLOCK_HOST;
        ctx->max_run_time_ns = SCALE_MS >> 7; /* XXX not sure how to calibrate this value */
    }

    ctx->deadline = timer_new_ns(ctx->limiter_clock, deadline_cb, ctx);
    timer_mod(ctx->deadline, qemu_clock_get_ns(ctx->limiter_clock) + ctx->max_run_time_ns);
}


int qemu_main(int argc, char const * argv[], char **envp);

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
        "-S",
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

    if (s->map_whole_as) {
        ctx->root_mr = map_io(ctx, get_system_memory(), 0, ((uint64_t)UINT32_MAX)+1);
    } else {
        ctx->root_mr = get_system_memory();
    }

    qemu_cond_init(&ctx->io_cond);

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

static QEMU_NORETURN void main_thread_abort(void)
{
    for (;;) {
        /* FIXME */
        qemu_cpu_cond_wait(&main_thread_ctx->io_cond);
    }
}

void QEMU_NORETURN __wrap_abort(void);
void QEMU_NORETURN __wrap_abort(void)
{
    main_thread_ctx->main_status = MAIN_ABORT;
    main_thread_abort();
}

void QEMU_NORETURN __wrap_exit(int status);
void QEMU_NORETURN __wrap_exit(int status)
{
    if (!status) {
        main_thread_ctx->main_status = MAIN_EXIT;
    } else {
        main_thread_ctx->main_status = MAIN_ABORT;
    }

    main_thread_abort();
}

