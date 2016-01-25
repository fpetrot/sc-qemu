#include <glib.h>

#include "qemu-common.h"
#include "qemu/main-loop.h"
#include "qemu/timer.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "exec/gdbstub.h"

#include "sc_qemu.h"
#include "sc_machine.h"
#include "qemu_context_priv.h"
#include "sc_qdev_priv.h"

qemu_context* SC_QEMU_INIT_SYM(sc_qemu_init_struct *s);

typedef struct mmio_ctx mmio_ctx;

struct mmio_ctx {
    qemu_context *qemu_ctx;
    uint32_t base;
};

typedef void (*ctor_fn)(void);
static GArray *ctor_fns = NULL;
static size_t ctor_count = 0;

static uint64_t sc_mmio_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    mmio_ctx *ctx = (mmio_ctx*) opaque;

    return ctx->qemu_ctx->sysc.read(ctx->qemu_ctx->opaque,
                                    ctx->base + offset, size);
}

static void sc_mmio_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
    mmio_ctx *ctx = (mmio_ctx*) opaque;

    ctx->qemu_ctx->sysc.write(ctx->qemu_ctx->opaque,
                              ctx->base + offset, value, size);
}

static const MemoryRegionOps sc_mmio_ops = {
    .read = sc_mmio_read,
    .write = sc_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


/* vl.c */
bool main_loop_should_exit(void);
/* ---- */

static bool sc_qemu_cpu_loop(qemu_context *ctx, int64_t *elapsed)
{
    int64_t before;

    if (elapsed) {
        before = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    }

    main_loop_wait(true);

    if (elapsed) {
        *elapsed = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - before;
    }

    return main_loop_should_exit();
}

static sc_qemu_qdev* sc_qemu_cpu_get_qdev(qemu_context *ctx, int cpu_idx)
{
    sc_qemu_qdev *ret = NULL;

    ret = g_malloc(sizeof(sc_qemu_qdev));

    ret->ctx = ctx;
    ret->id = SC_QDEV_CPU;
    ret->dev = DEVICE(ctx->cpus[cpu_idx]);

    return ret;
}

static void sc_qemu_map_io(qemu_context *ctx, uint32_t base_address,
                           uint32_t size)
{
    MemoryRegion *sysmem = get_system_memory();
    MemoryRegion *mmio = g_new(MemoryRegion, 1);
    mmio_ctx *m_ctx = g_new(mmio_ctx, 1);

    m_ctx->base = base_address;
    m_ctx->qemu_ctx = ctx;

    memory_region_init_io(mmio, NULL, &sc_mmio_ops, m_ctx, "sc-mmio", size);
    memory_region_add_subregion(sysmem, base_address, mmio);
}

static void sc_qemu_map_dmi(qemu_context *ctx, uint32_t base_address,
                            uint32_t size, void* data)
{
    MemoryRegion *sysmem = get_system_memory();
    MemoryRegion *dmi = g_new(MemoryRegion, 1);
    static int dmi_count = 0;

    char name[] = "sc-dmi  ";
    snprintf(name, sizeof(name), "sc-dmi%d", dmi_count);
    dmi_count++;

    memory_region_init_ram_ptr(dmi, NULL, name, size, data);
    vmstate_register_ram_global(dmi);
    memory_region_add_subregion(sysmem, base_address, dmi);
}

static void sc_qemu_start_gdbserver(qemu_context *ctx, const char *port)
{
    gdbserver_start(port);
    qemu_system_debug_request();
}

int qemu_main(int argc, char const * argv[], char **envp);


static void deadline_cb(void *opaque)
{
    qemu_context *ctx = (qemu_context *) opaque;

    timer_mod(ctx->deadline, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + ctx->max_run_time);
}

qemu_context* SC_QEMU_INIT_SYM(sc_qemu_init_struct *s)
{
    char num_cpu[4];
    size_t i;
    ctor_fn ctor;

    snprintf(num_cpu, sizeof(num_cpu), "%d", s->num_cpu);

    char const * qemu_base_argv[] = {
        "",
        "-M", "sc-qemu",
        "-cpu", s->cpu_model,
        "-smp", num_cpu,
        "-nographic",
    };

    char icount_option[] = "shift=00,align=off";

    char const * qemu_argv[ARRAY_SIZE(qemu_base_argv) + 2] = { 0 };
    int qemu_argc = ARRAY_SIZE(qemu_base_argv);

    memcpy(qemu_argv, qemu_base_argv, sizeof(qemu_base_argv));

    if (s->max_run_time > 0) {
        /* Enable icount */
        snprintf(icount_option, strlen(icount_option), "shift=%02d,align=off", s->cpu_mips_shift);
        qemu_argv[qemu_argc++] = "-icount";
        qemu_argv[qemu_argc++] = icount_option;
    }

    qemu_context *ctx;

    for (i = 0; i < ctor_count; i++) {
        ctor = g_array_index(ctor_fns, ctor_fn, i);
        ctor();
    }
    g_array_free(ctor_fns, TRUE);
    ctor_fns = NULL;
    ctor_count = 0;

    qemu_main(qemu_argc, qemu_argv, NULL);

    /* SystemC to QEMU fonctions */
    s->q_import->cpu_loop = sc_qemu_cpu_loop;
    s->q_import->cpu_get_qdev = sc_qemu_cpu_get_qdev;
    s->q_import->map_io = sc_qemu_map_io;
    s->q_import->map_dmi = sc_qemu_map_dmi;
    s->q_import->start_gdbserver = sc_qemu_start_gdbserver;
    s->q_import->qdev_create = sc_qemu_qdev_create;
    s->q_import->qdev_mmio_map = sc_qemu_qdev_mmio_map;
    s->q_import->qdev_irq_connect = sc_qemu_qdev_irq_connect;
    s->q_import->qdev_irq_update = sc_qemu_qdev_irq_update;
    s->q_import->char_dev_create = sc_qemu_char_dev_create;
    s->q_import->char_dev_write = sc_qemu_char_dev_write;
    s->q_import->char_dev_register_read = sc_qemu_char_dev_register_read;

    ctx = sc_qemu_machine_get_context();

    ctx->opaque = s->opaque;
    ctx->num_cpu = s->num_cpu;

    /* QEMU to SystemC functions */
    memcpy(&(ctx->sysc), &(s->sc_import), sizeof(systemc_import));

    if (s->max_run_time > 0) {
        /* Limit the number of instructions QEMU executes when calling the main loop */
        ctx->deadline = timer_new_ns(QEMU_CLOCK_VIRTUAL, deadline_cb, ctx);
        ctx->max_run_time = s->max_run_time;
        timer_mod(ctx->deadline, qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + ctx->max_run_time);
    }

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
