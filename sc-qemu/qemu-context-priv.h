#ifndef _RABBITS_QEMU_CONTEXT_H
#define _RABBITS_QEMU_CONTEXT_H

#include "sysemu/sysemu.h"
#include "qemu/typedefs.h"
#include "qemu/thread.h"

#include "sc-qemu.h"

enum ScQemuMainLoopStatus {
    MAIN_OK,
    MAIN_EXIT,
    MAIN_ABORT,
};

struct qemu_context {
    void *opaque;
    systemc_import sysc;

    QEMUClockType limiter_clock;
    int64_t max_run_time_ns;
    QEMUTimer *deadline;

    MemoryRegion *root_mr;

    enum ScQemuMainLoopStatus main_status;
    int64_t last_elapsed;
    QemuThread main_thread;
    QemuMutex mutex_sc, mutex_main;
};

#endif
