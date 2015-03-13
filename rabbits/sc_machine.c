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

static void sc_qemu_machine_init(MachineState *machine)
{
}

static QEMUMachine sc_qemu_machine = {
    .name = "sc-qemu",
    .desc = "Rabbits SystemC/QEMU dummy machine",
    .init = sc_qemu_machine_init,
};

static void sc_qemu_regiter_type(void)
{
    qemu_register_machine(&sc_qemu_machine);
}

machine_init(sc_qemu_regiter_type)

