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

typedef qemu_context* (*sc_qemu_init_fn)(qemu_import *, systemc_import *, void *);

typedef void (*sc_qemu_cpu_loop_fn)(qemu_context *);
typedef void (*sc_qemu_irq_update_fn)(qemu_context *, int cpu_mask, int level);
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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
