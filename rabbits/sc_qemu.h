#ifndef _RABBITS_SC_QEMU_EXPORT_H
#define _RABBITS_SC_QEMU_EXPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _strify(a) #a
#define strify(a) _strify(a)

#define SC_QEMU_INIT_SYM     sc_qemu_init
#define SC_QEMU_INIT_SYM_STR strify(SC_QEMU_INIT_SYM)

typedef struct qemu_context qemu_context;

typedef struct qemu_import qemu_import;
typedef struct systemc_import systemc_import;
typedef struct sc_qemu_init_struct sc_qemu_init_struct;

typedef qemu_context* (*sc_qemu_init_fn)(sc_qemu_init_struct *);

typedef bool (*sc_qemu_cpu_loop_fn)(qemu_context *);
typedef void (*sc_qemu_irq_update_fn)(qemu_context *, int cpu_idx, int irq_idx, int level);
typedef void (*sc_qemu_map_io_fn)(qemu_context *, uint32_t base_address, uint32_t size);
typedef void (*sc_qemu_map_dmi_fn)(qemu_context *, uint32_t base_address, uint32_t size, void *data);

typedef uint32_t (*qemu_sc_read_fn)(void *opaque, uint32_t addr, uint32_t size);
typedef void (*qemu_sc_write_fn)(void *opaque, uint32_t addr, uint32_t val, uint32_t size);

struct qemu_import {
    sc_qemu_cpu_loop_fn   cpu_loop;
    sc_qemu_irq_update_fn irq_update;
    sc_qemu_map_io_fn     map_io;
    sc_qemu_map_dmi_fn    map_dmi;
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

/* Targets specific */
#include "sc_qemu_arm.h"
/* ... add others here .. */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
