#include "hw/sysbus.h"
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
#include "target/target.h"

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

static void sc_qemu_machine_init(MachineState *machine)
{
    qemu_context *ctx;

    ctx = g_malloc0(sizeof(qemu_context));

    sc_qemu_machine_init_target(ctx, machine->cpu_model);

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

