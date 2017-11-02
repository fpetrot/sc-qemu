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

#include "sc-object.h"

struct qemu_import {
    sc_qemu_cpu_loop_fn                cpu_loop;              /* < Run the CPUs */
    sc_qemu_map_io_fn                  map_io;                /* < Map a memory area as io */
    sc_qemu_map_dmi_fn                 map_dmi;               /* < Map a memory area with direct memory access */
    sc_qemu_start_gdbserver_fn         start_gdbserver;       /* < Start a gdb server on the given port */

    sc_qemu_object_new_fn               object_new;               /* < Create a new QEMU object */
    sc_qemu_object_property_set_bool_fn object_property_set_bool; /* < Set a bool property on object */
    sc_qemu_object_property_set_int_fn  object_property_set_int;  /* < Set a int property on object */
    sc_qemu_object_property_set_str_fn  object_property_set_str;  /* < Set a string property on object */
    sc_qemu_object_property_set_link_fn object_property_set_link; /* < Set a link property on an object */

    sc_qemu_object_mmio_map_fn          object_mmio_map;          /* < Map a qdev memory area */
    sc_qemu_object_gpio_connect_fn      object_gpio_connect;      /* < Connect two gpios together */
    sc_qemu_object_gpio_update_fn       object_gpio_update;       /* < Set the value of a input gpio */
    sc_qemu_object_gpio_register_cb_fn  object_gpio_register_cb;  /* < Register a callback on out gpio value change */

    sc_qemu_cpu_get_id_fn               cpu_get_id;               /* < Get the CPU id of the corresponding sc_qemu_object */
    sc_qemu_object_get_root_mr_fn       object_get_root_mr;       /* < Get the root MemoryRegion as an sc_qemu_object */
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

    int64_t          max_run_time;  /* < [in]  Maximum cpus step in ns of the QEMU virtual clock. 0 is no limit */
    int              cpu_mips_shift;/* < [in]  QEMU icount shift option */
    void             *opaque;       /* < [in]  Opaque used as parameter of SystemC cb */

    bool             map_whole_as;  /* < [in]  If true, the whole address space is mapped
                                               so that SystemC can intercept non-mapped memory access. */
};

struct sc_qemu_io_attr {
    int cpuid;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
