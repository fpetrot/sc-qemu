#ifndef _RABBITS_SC_QDEV_H
#define _RABBITS_SC_QDEV_H

#include "typedefs.h"

sc_qemu_qdev* sc_qemu_qdev_create(qemu_context *ctx, int devid, ...);
void sc_qemu_qdev_mmio_map(sc_qemu_qdev *dev, int mmio_id, uint32_t addr);
void sc_qemu_qdev_irq_connect(sc_qemu_qdev *src, int src_idx, sc_qemu_qdev *dst, int dst_idx);
void sc_qemu_qdev_irq_update(sc_qemu_qdev *dev, int irq_idx, int level);
void sc_qemu_qdev_connect_gpio_out(sc_qemu_qdev *dev, int gpio_idx, void (*handler)(void *opaque, int n, int level), void *opaque);

#endif
