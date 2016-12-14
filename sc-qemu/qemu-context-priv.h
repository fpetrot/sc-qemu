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
    MAIN_MMIO_READ,
    MAIN_MMIO_WRITE
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

    /* MMIO */
    uint64_t mmio_addr;
    uint64_t mmio_value;
    unsigned mmio_size;
    sc_qemu_io_attr mmio_attr;
};

#endif
