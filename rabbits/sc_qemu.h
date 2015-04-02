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


typedef struct qemu_context qemu_context;
typedef struct sc_qemu_qdev sc_qemu_qdev;

typedef struct qemu_import qemu_import;
typedef struct systemc_import systemc_import;
typedef struct sc_qemu_init_struct sc_qemu_init_struct;

typedef qemu_context* (*sc_qemu_init_fn)(sc_qemu_init_struct *);

typedef enum sc_qemu_qdev_e {
    /* Variadic initialisation :
     *  - number of interrupts, uint32_t
     * MMIO Mapping :
     *  0: GIC  base address
     */
    SC_QDEV_A15PRIV = 0,

    /* Variadic initialisation :
     *  - base address, uint32_t
     *  - register shift, int
     *  - interrupt controller, sc_qemu_qdev*
     *  - irq line index, int
     *  - baud base, int
     */
    SC_QDEV_16550,

    /* Variadic initialisation :
     *  nothing
     * MMIO Mapping:
     *  0: sp804 base address
     */
    SC_QDEV_SP804,
} sc_qemu_qdev_e;

typedef bool (*sc_qemu_cpu_loop_fn)(qemu_context *);
typedef void (*sc_qemu_irq_update_fn)(qemu_context *, int cpu_idx, int irq_idx, int level);
typedef void (*sc_qemu_map_io_fn)(qemu_context *, uint32_t base_address, uint32_t size);
typedef void (*sc_qemu_map_dmi_fn)(qemu_context *, uint32_t base_address, uint32_t size, void *data);
typedef sc_qemu_qdev* (*sc_qemu_qdev_create_fn)(qemu_context *, sc_qemu_qdev_e, ...);
typedef void (*sc_qemu_qdev_mmio_map_fn)(sc_qemu_qdev *dev, int mmio_id, uint32_t addr);
typedef void (*sc_qemu_qdev_irq_connect_fn)(sc_qemu_qdev *src, int src_idx, sc_qemu_qdev *dst, int dst_idx);
typedef void (*sc_qemu_qdev_irq_update_fn)(sc_qemu_qdev *dev, int irq_idx, int level);

typedef uint32_t (*qemu_sc_read_fn)(void *opaque, uint32_t addr, uint32_t size);
typedef void (*qemu_sc_write_fn)(void *opaque, uint32_t addr, uint32_t val, uint32_t size);

#include "sc_qdev.h"
#include "sc_qemu_arm.h"

struct qemu_import {
    sc_qemu_cpu_loop_fn         cpu_loop;
    sc_qemu_irq_update_fn       irq_update;
    sc_qemu_map_io_fn           map_io;
    sc_qemu_map_dmi_fn          map_dmi;
    sc_qemu_qdev_create_fn      qdev_create;
    sc_qemu_qdev_mmio_map_fn    qdev_mmio_map;
    sc_qemu_qdev_irq_connect_fn qdev_irq_connect;
    sc_qemu_qdev_irq_update_fn  qdev_irq_update;
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
