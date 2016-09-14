#ifndef _RABBITS_SC_QDEV_PRIV_H
#define _RABBITS_SC_QDEV_PRIV_H

#include "sc-object.h"

struct sc_qemu_object {
    qemu_context   *ctx;
    Object         *obj;
};
#endif
