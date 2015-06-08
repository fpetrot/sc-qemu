#ifndef _RABBITS_TYPEDEFS_H
#define _RABBITS_TYPEDEFS_H

typedef struct qemu_context qemu_context;
typedef struct sc_qemu_qdev sc_qemu_qdev;

typedef struct qemu_import qemu_import;
typedef struct systemc_import systemc_import;
typedef struct sc_qemu_init_struct sc_qemu_init_struct;

typedef struct sc_qemu_char_dev sc_qemu_char_dev;

typedef enum sc_qemu_qdev_e {
    /* Cannot be created by qdev_create. Use cpu_get_qdev */
    SC_QDEV_CPU = 0,

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

    /* Keep it at the end */
    SC_QDEV_LAST,

} sc_qemu_qdev_e;

typedef qemu_context* (*sc_qemu_init_fn)(sc_qemu_init_struct *);

typedef bool (*sc_qemu_cpu_loop_fn)(qemu_context *);
typedef sc_qemu_qdev* (*sc_qemu_cpu_get_qdev_fn)(qemu_context *, int cpu_idx);
typedef void (*sc_qemu_map_io_fn)(qemu_context *, uint32_t base_address, uint32_t size);
typedef void (*sc_qemu_map_dmi_fn)(qemu_context *, uint32_t base_address, uint32_t size, void *data);
typedef void (*sc_qemu_start_gdbserver_fn)(qemu_context *, const char *port);

typedef sc_qemu_qdev* (*sc_qemu_qdev_create_fn)(qemu_context *, int qdev_id, ...);
typedef void (*sc_qemu_qdev_mmio_map_fn)(sc_qemu_qdev *dev, int mmio_id, uint32_t addr);
typedef void (*sc_qemu_qdev_irq_connect_fn)(sc_qemu_qdev *src, int src_idx, sc_qemu_qdev *dst, int dst_idx);
typedef void (*sc_qemu_qdev_irq_update_fn)(sc_qemu_qdev *dev, int irq_idx, int level);

typedef uint32_t (*qemu_sc_read_fn)(void *opaque, uint32_t addr, uint32_t size);
typedef void (*qemu_sc_write_fn)(void *opaque, uint32_t addr, uint32_t val, uint32_t size);

typedef sc_qemu_char_dev* (*sc_qemu_char_dev_create_fn)(qemu_context *);
typedef int (*sc_qemu_char_dev_write_fn)(sc_qemu_char_dev *dev, const uint8_t *data, int len);
typedef void (*sc_qemu_char_dev_read_fn)(void *opaque, const uint8_t *data, int len);
typedef void (*sc_qemu_char_dev_register_read_fn)(sc_qemu_char_dev *dev, sc_qemu_char_dev_read_fn, void *opaque);

#endif
