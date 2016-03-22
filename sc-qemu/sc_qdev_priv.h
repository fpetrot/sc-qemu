#ifndef _RABBITS_SC_QDEV_PRIV_H
#define _RABBITS_SC_QDEV_PRIV_H

#include "sc_qdev.h"

struct sc_qemu_qdev {
    qemu_context   *ctx;
    sc_qemu_qdev_e  id;
    DeviceState    *dev;
};

#endif
