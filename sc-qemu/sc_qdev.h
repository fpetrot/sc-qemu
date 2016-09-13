#ifndef _RABBITS_SC_QDEV_H
#define _RABBITS_SC_QDEV_H

#include "typedefs.h"

sc_qemu_qdev* sc_qemu_qdev_create(qemu_context *ctx, int devid, ...);
void sc_qemu_qdev_mmio_map(sc_qemu_qdev *dev, int mmio_id, uint32_t addr);
void sc_qemu_qdev_irq_connect(sc_qemu_qdev *src, int src_idx, sc_qemu_qdev *dst, int dst_idx);
void sc_qemu_qdev_irq_update(sc_qemu_qdev *dev, int irq_idx, int level);
void sc_qemu_qdev_gpio_register_cb(sc_qemu_qdev *dev, int gpio_idx,
                                   sc_qemu_qdev_gpio_cb_fn cb, void *opaque);


sc_qemu_object* sc_qemu_object_new(qemu_context *ctx, const char *type_name);
void sc_qemu_object_property_set_bool(sc_qemu_object *obj, bool val, const char *name);
void sc_qemu_object_property_set_int(sc_qemu_object *obj, int64_t val, const char *name);
void sc_qemu_object_property_set_str(sc_qemu_object *obj, const char *val, const char *name);
#endif
