#ifndef _RABBITS_TYPEDEFS_H
#define _RABBITS_TYPEDEFS_H

typedef struct qemu_context qemu_context;
typedef struct sc_qemu_object sc_qemu_object;

typedef struct qemu_import qemu_import;
typedef struct systemc_import systemc_import;
typedef struct sc_qemu_init_struct sc_qemu_init_struct;
typedef struct sc_qemu_io_attr sc_qemu_io_attr;

typedef struct sc_qemu_char_dev sc_qemu_char_dev;

typedef qemu_context* (*sc_qemu_init_fn)(sc_qemu_init_struct *);

typedef bool (*sc_qemu_cpu_loop_fn)(qemu_context *, int64_t *elapsed, bool *has_work);
typedef void (*sc_qemu_map_io_fn)(qemu_context *, uint32_t base_address, uint32_t size);
typedef void (*sc_qemu_map_dmi_fn)(qemu_context *, uint32_t base_address, uint32_t size, void *data, bool readonly);
typedef void (*sc_qemu_start_gdbserver_fn)(qemu_context *, const char *port);

typedef uint32_t (*qemu_sc_read_fn)(void *opaque, uint32_t addr, uint32_t size, const sc_qemu_io_attr *);
typedef void (*qemu_sc_write_fn)(void *opaque, uint32_t addr, uint32_t val, uint32_t size, const sc_qemu_io_attr *);

typedef sc_qemu_char_dev* (*sc_qemu_char_dev_create_fn)(qemu_context *);
typedef int (*sc_qemu_char_dev_write_fn)(sc_qemu_char_dev *dev, const uint8_t *data, int len);
typedef void (*sc_qemu_char_dev_read_fn)(void *opaque, const uint8_t *data, int len);
typedef void (*sc_qemu_char_dev_register_read_fn)(sc_qemu_char_dev *dev, sc_qemu_char_dev_read_fn, void *opaque);

typedef sc_qemu_object* (*sc_qemu_object_new_fn)(qemu_context *ctx, const char *type_name);
typedef void (*sc_qemu_object_property_set_bool_fn)(sc_qemu_object *obj, bool val, const char *name);
typedef void (*sc_qemu_object_property_set_int_fn)(sc_qemu_object *obj, int64_t val, const char *name);
typedef void (*sc_qemu_object_property_set_str_fn)(sc_qemu_object *obj, const char * val, const char *name);
typedef void (*sc_qemu_object_property_set_link_fn)(sc_qemu_object *obj, sc_qemu_object * link, const char *name);

typedef void (*sc_qemu_object_mmio_map_fn)(sc_qemu_object *obj, int mmio_id, uint32_t addr);

typedef void (*sc_qemu_object_gpio_connect_fn)(sc_qemu_object *src, const char *src_name, int src_idx,
                                               sc_qemu_object *dst, const char *dst_name, int dst_idx);

typedef void (*sc_qemu_object_gpio_update_fn)(sc_qemu_object *obj, const char *gpio_name,
                                              int gpio_idx, int level);

typedef void (*sc_qemu_object_gpio_cb_fn)(sc_qemu_object *obj, int n, int level, void *opaque);

typedef void (*sc_qemu_object_gpio_register_cb_fn)(sc_qemu_object *obj, const char *gpio_name, int gpio_idx,
                                                   sc_qemu_object_gpio_cb_fn cb, void *opaque);

typedef int (*sc_qemu_cpu_get_id_fn)(sc_qemu_object *obj);
typedef sc_qemu_object* (*sc_qemu_object_get_root_mr_fn)(qemu_context *ctx);

#endif
