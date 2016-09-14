#include "qemu/osdep.h"
#include "qemu-common.h"
#include "hw/sysbus.h"
#include "sysemu/sysemu.h"
#include "sysemu/char.h"
#include "hw/char/serial.h"
#include "exec/address-spaces.h"
#include "qapi/error.h"
#include "qom/cpu.h"

#include "sc-object-priv.h"
#include "qemu-context-priv.h"

struct gpio_cb_descr {
    sc_qemu_object_gpio_cb_fn cb;
    sc_qemu_object *obj;
    void *opaque;
};

sc_qemu_object * sc_qemu_object_new(qemu_context *ctx, const char *typename)
{
    sc_qemu_object *ret = NULL;

    ret = g_malloc0(sizeof(sc_qemu_object));
    ret->ctx = ctx;

    ret->obj = object_new(typename);

    return ret;
}

void sc_qemu_object_property_set_bool(sc_qemu_object *obj, bool val, const char *name)
{
    Object *o = obj->obj;

    assert(object_property_find(o, name, NULL));
    object_property_set_bool(o, val, name, &error_abort);
}

void sc_qemu_object_property_set_int(sc_qemu_object *obj, int64_t val, const char *name)
{
    Object *o = obj->obj;

    assert(object_property_find(o, name, NULL));
    object_property_set_int(o, val, name, &error_abort);
}

void sc_qemu_object_property_set_str(sc_qemu_object *obj, const char *val, const char *name)
{
    Object *o = obj->obj;

    assert(object_property_find(o, name, NULL));
    object_property_set_str(o, val, name, &error_abort);
}

int sc_qemu_cpu_get_id(sc_qemu_object *obj)
{
    CPUState *cpu = (CPUState*) object_dynamic_cast(obj->obj, TYPE_CPU);

    assert(cpu != NULL);

    return cpu->cpu_index;
}

void sc_qemu_object_mmio_map(sc_qemu_object *obj, int mmio_id, uint32_t addr)
{
    sysbus_mmio_map(SYS_BUS_DEVICE(obj->obj), mmio_id, addr);
}

void sc_qemu_object_gpio_connect(sc_qemu_object *src, const char *src_name, int src_idx,
                                 sc_qemu_object *dst, const char *dst_name, int dst_idx)
{
    qemu_irq gpio_in = qdev_get_gpio_in_named(DEVICE(dst->obj), dst_name, dst_idx);
    qdev_connect_gpio_out_named(DEVICE(src->obj), src_name, src_idx, gpio_in);
}

void sc_qemu_object_gpio_update(sc_qemu_object *obj, const char *gpio_name, int gpio_idx, int level)
{
    qemu_irq i = qdev_get_gpio_in_named(DEVICE(obj->obj), gpio_name, gpio_idx);
    qemu_set_irq(i, level);
}

static void object_gpio_cb(void *opaque, int n, int level)
{
    struct gpio_cb_descr *descr = (struct gpio_cb_descr*) opaque;
    descr->cb(descr->obj, n, level, descr->opaque);
}

void sc_qemu_object_gpio_register_cb(sc_qemu_object *obj, const char *gpio_name, int gpio_idx,
                                     sc_qemu_object_gpio_cb_fn cb, void *opaque)
{
    struct gpio_cb_descr *descr = g_malloc(sizeof(struct gpio_cb_descr));
    descr->cb = cb;
    descr->obj = obj;
    descr->opaque = opaque;

    qemu_irq interceptor = qemu_allocate_irq(object_gpio_cb, (void*) descr, 1);
    qdev_connect_gpio_out_named(DEVICE(obj->obj), gpio_name, gpio_idx, interceptor);
}

