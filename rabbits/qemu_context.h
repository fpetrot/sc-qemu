#ifndef _RABBITS_QEMU_CONTEXT_H
#define _RABBITS_QEMU_CONTEXT_H

#include "sc_qemu.h"

struct qemu_context {
    void *opaque;
    systemc_import sysc;
    int num_cpu;
    ARMCPU **cpus;
};

#endif
