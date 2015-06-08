#ifndef _RABBITS_QEMU_CONTEXT_H
#define _RABBITS_QEMU_CONTEXT_H

#include "sysemu/sysemu.h"

#include "sc_qemu.h"

struct qemu_context {
    void *opaque;
    systemc_import sysc;
    int num_cpu;
    CPUState **cpus;
};

#endif
