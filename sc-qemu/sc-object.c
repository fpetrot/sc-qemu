#include "qemu/osdep.h"
#include "qemu-common.h"
#include "hw/sysbus.h"
#include "sysemu/sysemu.h"
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

static inline void lock_iothread(bool was_locked)
{
    if (!was_locked) {
        qemu_mutex_lock_iothread();
    }
}

static inline void unlock_iothread(bool was_locked)
{
    if (!was_locked) {
        qemu_mutex_unlock_iothread();
    }
}

/* XXX Not sure how to do that properly */
static void register_reset(Object *obj)
{
    if (object_dynamic_cast(obj, TYPE_CPU)) {
        qemu_register_reset((void (*)(void*))cpu_reset, obj);
    } else if (object_dynamic_cast(obj, TYPE_DEVICE)) {
        qemu_register_reset(qdev_reset_all_fn, obj);
    }
}

sc_qemu_object * sc_qemu_object_new(qemu_context *ctx, const char *typename)
{
    sc_qemu_object *ret = NULL;

    ret = g_malloc0(sizeof(sc_qemu_object));
    ret->ctx = ctx;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    ret->obj = object_new(typename);
    object_property_add_child(object_get_root(), "sc-qemu-object[*]", ret->obj, &error_abort);

    register_reset(ret->obj);

    unlock_iothread(was_locked);

    return ret;
}

void sc_qemu_object_property_set_bool(sc_qemu_object *obj, bool val, const char *name)
{
    Object *o = obj->obj;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    assert(object_property_find(o, name, NULL));
    object_property_set_bool(o, val, name, &error_abort);

    unlock_iothread(was_locked);
}

void sc_qemu_object_property_set_int(sc_qemu_object *obj, int64_t val, const char *name)
{
    Object *o = obj->obj;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    assert(object_property_find(o, name, NULL));
    object_property_set_int(o, val, name, &error_abort);

    unlock_iothread(was_locked);
}

void sc_qemu_object_property_set_str(sc_qemu_object *obj, const char *val, const char *name)
{
    Object *o = obj->obj;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    assert(object_property_find(o, name, NULL));
    object_property_set_str(o, val, name, &error_abort);

    unlock_iothread(was_locked);
}

void sc_qemu_object_property_set_link(sc_qemu_object *obj, sc_qemu_object *link, const char *name)
{
    Object *o = obj->obj;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    assert(object_property_find(o, name, NULL));
    object_property_set_link(o, link->obj, name, &error_abort);

    unlock_iothread(was_locked);
}

int sc_qemu_cpu_get_id(sc_qemu_object *obj)
{
    CPUState *cpu = (CPUState*) object_dynamic_cast(obj->obj, TYPE_CPU);

    assert(cpu != NULL);

    return cpu->cpu_index;
}

sc_qemu_object * sc_qemu_object_get_root_mr(qemu_context *ctx)
{
    return &ctx->root_mr;
}

void sc_qemu_object_mmio_map(sc_qemu_object *obj, int mmio_id, uint32_t addr)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    sysbus_mmio_map(SYS_BUS_DEVICE(obj->obj), mmio_id, addr);

    unlock_iothread(was_locked);
}

void sc_qemu_object_gpio_connect(sc_qemu_object *src, const char *src_name, int src_idx,
                                 sc_qemu_object *dst, const char *dst_name, int dst_idx)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    qemu_irq gpio_in = qdev_get_gpio_in_named(DEVICE(dst->obj), dst_name, dst_idx);
    qdev_connect_gpio_out_named(DEVICE(src->obj), src_name, src_idx, gpio_in);

    unlock_iothread(was_locked);
}

void sc_qemu_object_gpio_update(sc_qemu_object *obj, const char *gpio_name, int gpio_idx, int level)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    qemu_irq i = qdev_get_gpio_in_named(DEVICE(obj->obj), gpio_name, gpio_idx);
    qemu_set_irq(i, level);

    unlock_iothread(was_locked);
}

static void object_gpio_cb(void *opaque, int n, int level)
{
    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    struct gpio_cb_descr *descr = (struct gpio_cb_descr*) opaque;
    descr->cb(descr->obj, n, level, descr->opaque);

    unlock_iothread(was_locked);
}

void sc_qemu_object_gpio_register_cb(sc_qemu_object *obj, const char *gpio_name, int gpio_idx,
                                     sc_qemu_object_gpio_cb_fn cb, void *opaque)
{
    struct gpio_cb_descr *descr = g_malloc(sizeof(struct gpio_cb_descr));
    descr->cb = cb;
    descr->obj = obj;
    descr->opaque = opaque;

    bool was_locked = qemu_mutex_iothread_locked();
    lock_iothread(was_locked);

    qemu_irq interceptor = qemu_allocate_irq(object_gpio_cb, (void*) descr, 1);
    qdev_connect_gpio_out_named(DEVICE(obj->obj), gpio_name, gpio_idx, interceptor);

    unlock_iothread(was_locked);
}

