#ifndef _RABBITS_SC_QDEV_H
#define _RABBITS_SC_QDEV_H

#include "typedefs.h"

sc_qemu_object* sc_qemu_object_new(qemu_context *ctx, const char *type_name);

void sc_qemu_object_property_set_bool(sc_qemu_object *obj, bool val, const char *name);
void sc_qemu_object_property_set_int(sc_qemu_object *obj, int64_t val, const char *name);
void sc_qemu_object_property_set_str(sc_qemu_object *obj, const char *val, const char *name);
void sc_qemu_object_property_set_link(sc_qemu_object *obj, sc_qemu_object *link, const char *name);

void sc_qemu_object_mmio_map(sc_qemu_object *obj, int mmio_id, uint32_t addr);

void sc_qemu_object_gpio_connect(sc_qemu_object *src, const char *src_name, int src_idx,
                                 sc_qemu_object *dst, const char *dst_name, int dst_idx);

void sc_qemu_object_gpio_update(sc_qemu_object *obj, const char *gpio_name,
                                int gpio_idx, int level);

void sc_qemu_object_gpio_register_cb(sc_qemu_object *obj, const char *gpio_name, int gpio_idx, 
                                     sc_qemu_object_gpio_cb_fn cb, void *opaque);

int sc_qemu_cpu_get_id(sc_qemu_object *obj);
sc_qemu_object * sc_qemu_object_get_root_mr(qemu_context *ctx);

#endif
