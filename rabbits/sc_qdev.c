#include "qemu-common.h"
#include "hw/sysbus.h"
#include "sysemu/sysemu.h"
#include "hw/char/serial.h"
#include "exec/address-spaces.h"

#include "sc_qdev.h"
#include "qemu_context.h"

struct sc_qemu_qdev {
    qemu_context   *ctx;
    sc_qemu_qdev_e  id;
    DeviceState    *dev;
};

sc_qemu_qdev* sc_qemu_qdev_create(qemu_context *ctx, sc_qemu_qdev_e devid, ...)
{
    sc_qemu_qdev *ret = NULL;
    va_list ap;
    uint32_t u;
    int i;

    va_start(ap, devid);

    ret = g_malloc0(sizeof(sc_qemu_qdev));
    ret->ctx = ctx;
    ret->id = devid;

    switch (devid) {
    case SC_QDEV_A15PRIV:
        u = va_arg(ap, uint32_t); /* num-irq property */
        ret->dev = qdev_create(NULL, "a15mpcore_priv");
        

        qdev_prop_set_uint32(ret->dev, "num-cpu", ctx->num_cpu);
        qdev_prop_set_uint32(ret->dev, "num-irq", u);

        qdev_init_nofail(ret->dev);

        for (i = 0; i < ctx->num_cpu; i++) {
            DeviceState *cpudev = DEVICE(ctx->cpus[i]);
            sysbus_connect_irq(SYS_BUS_DEVICE(ret->dev), i, qdev_get_gpio_in(cpudev, ARM_CPU_IRQ));
        }

        break;

    case SC_QDEV_16550:
        {
            uint32_t base_addr = va_arg(ap, uint32_t);
            int regshift = va_arg(ap, int);
            sc_qemu_qdev *int_ctrl = va_arg(ap, sc_qemu_qdev*);
            int irq_idx = va_arg(ap, int);
            int baudbase = va_arg(ap, int);

            serial_mm_init(get_system_memory(), base_addr, regshift,
                           qdev_get_gpio_in(int_ctrl->dev, irq_idx), baudbase,
                           serial_hds[0], DEVICE_NATIVE_ENDIAN);
        }
        break;

    case SC_QDEV_SP804:
        ret->dev = qdev_create(NULL, "sp804");
        qdev_init_nofail(ret->dev);
        break;
    }

    va_end(ap);

    return ret;
}

void sc_qemu_qdev_mmio_map(sc_qemu_qdev *dev, int mmio_id, uint32_t addr)
{
    sysbus_mmio_map(SYS_BUS_DEVICE(dev->dev), mmio_id, addr);
}

void sc_qemu_qdev_irq_connect(sc_qemu_qdev *src, int src_idx,
                              sc_qemu_qdev *dst, int dst_idx)
{
    sysbus_connect_irq(SYS_BUS_DEVICE(dst->dev), dst_idx, qdev_get_gpio_in(src->dev, src_idx));
}

void sc_qemu_qdev_irq_update(sc_qemu_qdev *dev, int irq_idx, int level)
{
    qemu_irq i = qdev_get_gpio_in(dev->dev, irq_idx);
    qemu_set_irq(i, level);
}

