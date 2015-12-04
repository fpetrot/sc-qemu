#include "qemu-common.h"
#include "hw/sysbus.h"
#include "sysemu/sysemu.h"
#include "sysemu/char.h"
#include "hw/char/serial.h"
#include "exec/address-spaces.h"

#include "sc_qdev_priv.h"
#include "qemu_context_priv.h"
#include "target/target.h"

sc_qemu_qdev* sc_qemu_qdev_create(qemu_context *ctx, int devid, ...)
{
    sc_qemu_qdev *ret = NULL;
    va_list ap;
    static int chr_idx = 0;

    va_start(ap, devid);

    ret = g_malloc0(sizeof(sc_qemu_qdev));
    ret->ctx = ctx;
    ret->id = devid;

    switch (devid) {
    case SC_QDEV_16550:
        {
            uint32_t regshift = va_arg(ap, uint32_t);
            uint32_t baudbase = va_arg(ap, uint32_t);

            ret->dev = qdev_create(NULL, "serial");

            qdev_prop_set_uint32(ret->dev, "it-shift", regshift);
            qdev_prop_set_uint32(ret->dev, "baudbase", baudbase);
            qdev_prop_set_chr(ret->dev, "chardev", serial_hds[chr_idx++]);

            qdev_init_nofail(ret->dev);
        }
        break;

    case SC_QDEV_SP804:
        ret->dev = qdev_create(NULL, "sp804");
        qdev_init_nofail(ret->dev);
        break;

    default:
        sc_qemu_target_qdev_create(ret, devid, ap);
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
    if (src->id == SC_QDEV_CPU) {
        qdev_connect_gpio_out(src->dev, src_idx, qdev_get_gpio_in(dst->dev, dst_idx));
    } else {
        sysbus_connect_irq(SYS_BUS_DEVICE(src->dev), src_idx, qdev_get_gpio_in(dst->dev, dst_idx));
    }
}

void sc_qemu_qdev_irq_update(sc_qemu_qdev *dev, int irq_idx, int level)
{
    qemu_irq i = qdev_get_gpio_in(dev->dev, irq_idx);
    qemu_set_irq(i, level);
}

