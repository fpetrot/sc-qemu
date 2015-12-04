/*
 * QOM version of the 16550
 *
 * Copyright (c) 2015 Luc Michel.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hw/sysbus.h"
#include "hw/char/serial.h"


#define SERIAL(obj) OBJECT_CHECK(SerialQomState, (obj), \
                                       TYPE_SERIAL)
struct SerialQomState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    uint32_t baudbase;
    uint32_t it_shift;
    struct SerialState s;
};

typedef struct SerialQomState SerialQomState;

static void serial_qom_realize(DeviceState *dev, Error **errp)
{
    SerialQomState *qom_s = SERIAL(dev);
    SerialState *s = &qom_s->s;
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    Error *err = NULL;

    s->baudbase = qom_s->baudbase;
    s->it_shift = qom_s->it_shift;

    serial_realize_core(s, &err);
    if(err != NULL) {
        error_propagate(errp, err);
        return;
    }

    memory_region_init_io(&s->io, OBJECT(dev), serial_mm_ops, s, "serial", 8 << s->it_shift);
    sysbus_init_mmio(sbd, &s->io);
    sysbus_init_irq(sbd, &s->irq);
}

static Property serial_qom_properties[] = {
    DEFINE_PROP_UINT32("baudbase", SerialQomState, baudbase, 115200),
    DEFINE_PROP_UINT32("it-shift", SerialQomState, it_shift, 2),
    DEFINE_PROP_CHR("chardev", SerialQomState, s.chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void serial_qom_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = serial_qom_realize;
    dc->vmsd = &vmstate_serial;
    dc->props = serial_qom_properties;
}

static const TypeInfo serial_qom_info = {
    .name          = TYPE_SERIAL,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SerialQomState),
    .class_init    = serial_qom_class_init,
};

static void serial_qom_register_types(void)
{
    type_register_static(&serial_qom_info);
}

type_init(serial_qom_register_types)
