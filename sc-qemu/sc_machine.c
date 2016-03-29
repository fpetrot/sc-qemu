#include "qemu/osdep.h"
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

static void sc_qemu_machine_class_init(MachineClass *mc)
{
    mc->desc = "Rabbits SystemC/QEMU machine";
    mc->init = sc_qemu_machine_init;
    mc->max_cpus = 128;
}

DEFINE_MACHINE("sc-qemu", sc_qemu_machine_class_init)

