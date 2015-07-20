#include "qemu-common.h"
#include "qemu/main-loop.h"
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

static bool sc_qemu_cpu_loop(qemu_context *ctx)
{
    main_loop_wait(true);

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

qemu_context* SC_QEMU_INIT_SYM(sc_qemu_init_struct *s)
{
    char num_cpu[4];

    snprintf(num_cpu, sizeof(num_cpu), "%d", s->num_cpu);

    char const * qemu_argv[] = {
        "",
        "-M", "sc-qemu",
        "-cpu", s->cpu_model,
        "-smp", num_cpu,
#if 0
        "-serial", "stdio",
#else
        "-nographic",
#endif
        /*"-d", "in_asm,exec",*/
        /*"-D", "qemu.log",*/
    };

    qemu_context *ctx;
    int qemu_argc = ARRAY_SIZE(qemu_argv);

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

    return ctx;
}
