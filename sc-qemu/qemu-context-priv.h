#ifndef _RABBITS_QEMU_CONTEXT_H
#define _RABBITS_QEMU_CONTEXT_H

#include "sysemu/sysemu.h"
#include "qemu/typedefs.h"
#include "qemu/thread.h"
#include "qemu/coroutine.h"

#include "sc-qemu.h"
#include "sc-object-priv.h"

enum ScQemuMainLoopStatus {
    MAIN_OK,
    MAIN_EXIT,
    MAIN_ABORT,
    MAIN_MMIO_READ,
    MAIN_MMIO_WRITE
};

struct qemu_context {
    void *opaque;
    systemc_import sysc;

    bool elaboration_done;

    int64_t max_run_time_ns;

    sc_qemu_object root_mr;

    enum ScQemuMainLoopStatus main_status;
};

#endif
