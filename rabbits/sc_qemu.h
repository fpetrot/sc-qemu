#ifndef _RABBITS_SC_QEMU_EXPORT_H
#define _RABBITS_SC_QEMU_EXPORT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _strify(a) #a
#define strify(a) _strify(a)

#define SC_QEMU_INIT_SYM     sc_qemu_init
#define SC_QEMU_INIT_SYM_STR strify(SC_QEMU_INIT_SYM)


#include "typedefs.h"

#include "sc_qdev.h"
#include "sc_qemu_char_dev.h"
#include "target/arm.h"

struct qemu_import {
    sc_qemu_cpu_loop_fn         cpu_loop;           /* < Run the CPUs */
    sc_qemu_cpu_get_qdev_fn     cpu_get_qdev;       /* < Get the qdev corresponding to a cpu */
    sc_qemu_map_io_fn           map_io;             /* < Map a memory area as io */
    sc_qemu_map_dmi_fn          map_dmi;            /* < Map a memory area with direct memory access */
    sc_qemu_start_gdbserver_fn  start_gdbserver;    /* < Start a gdb server on the given port */
    sc_qemu_qdev_create_fn      qdev_create;        /* < Create a new QEMU device */
    sc_qemu_qdev_mmio_map_fn    qdev_mmio_map;      /* < Map a qdev to a memory area */
    sc_qemu_qdev_irq_connect_fn qdev_irq_connect;   /* < Connect irq lines of two qdevs */
    sc_qemu_qdev_irq_update_fn  qdev_irq_update;    /* < Update a qdev input irq line */

    sc_qemu_char_dev_create_fn          char_dev_create;        /* < Create a qemu char device */
    sc_qemu_char_dev_write_fn           char_dev_write;         /* < Write to a qemu char device */
    sc_qemu_char_dev_register_read_fn   char_dev_register_read; /* < Register a read callback for a qemu char device */
};

struct systemc_import {
    qemu_sc_read_fn  read;
    qemu_sc_write_fn write;
};

struct sc_qemu_init_struct {
    qemu_import      *q_import;     /* < [out] Filled by QEMU after init call.
                                     *  This field is not allocated by QEMU and
                                     *  must target a valid address before the init call */
    systemc_import   sc_import;     /* < [in]  SystemC callbacks used by QEMU  */
    const char       *cpu_model;    /* < [in]  Requested cpu model */
    int              num_cpu;       /* < [in]  Requested number of cpus */
    void             *opaque;       /* < [in]  Opaque used as parameter of SystemC cb */
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
