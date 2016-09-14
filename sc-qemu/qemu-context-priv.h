#ifndef _RABBITS_QEMU_CONTEXT_H
#define _RABBITS_QEMU_CONTEXT_H

#include "sysemu/sysemu.h"
#include "qemu/typedefs.h"

#include "sc-qemu.h"

struct qemu_context {
    void *opaque;
    systemc_import sysc;
    QEMUTimer *deadline;
    int64_t max_run_time;
    MemoryRegion *root_mr;
};

#endif
