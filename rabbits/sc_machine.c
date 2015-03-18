#include "hw/sysbus.h"
#include "hw/arm/arm.h"
#include "hw/devices.h"
#include "net/net.h"
#include "sysemu/sysemu.h"
#include "hw/pci/pci.h"
#include "hw/i2c/i2c.h"
#include "hw/boards.h"
#include "sysemu/block-backend.h"
#include "exec/address-spaces.h"
#include "hw/block/flash.h"
#include "qemu/error-report.h"

#include "sc_machine.h"

/*
 * Used during QEMU initialisation only.
 * Since the machine is initialised by qemu_main, we need 
 * a way to get back the initialised context. This is this pointer 
 */
static qemu_context *inited_context = NULL;

qemu_context * sc_qemu_machine_get_context(void)
{
    qemu_context * ret = inited_context;
    inited_context = NULL;
    return ret;
}

static void init_arm_cpu(qemu_context *ctx, const char *model)
{
    ARMCPU *cpu;
    char const *m = model;
    int i;

    if(model == NULL) {
        m = "arm1176";
    }

    ctx->cpus = g_malloc0(sizeof(ARMCPU*) * smp_cpus);

    for(i = 0; i < smp_cpus; i++) {
        cpu = cpu_arm_init(m);
        if (!cpu) {
            fprintf(stderr, "Unable to find CPU definition\n");
            exit(1);
        }

        ctx->cpus[i] = cpu;
    }
}

static void sc_qemu_machine_init(MachineState *machine)
{
    qemu_context *ctx;

    ctx = g_malloc0(sizeof(qemu_context));

    init_arm_cpu(ctx, machine->cpu_model);

    inited_context = ctx;
}

static QEMUMachine sc_qemu_machine = {
    .name = "sc-qemu",
    .desc = "Rabbits SystemC/QEMU machine",
    .init = sc_qemu_machine_init,
    .max_cpus = 128,
};

static void sc_qemu_regiter_type(void)
{
    qemu_register_machine(&sc_qemu_machine);
}

machine_init(sc_qemu_regiter_type)

