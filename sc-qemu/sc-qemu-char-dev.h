#ifndef _RABBITS_SC_QEMU_CHAR_DEV_H
#define _RABBITS_SC_QEMU_CHAR_DEV_H

#include "typedefs.h"

sc_qemu_char_dev* sc_qemu_char_dev_create(qemu_context *);
int sc_qemu_char_dev_write(sc_qemu_char_dev *dev, const uint8_t *data, int len);
void sc_qemu_char_dev_register_read(sc_qemu_char_dev *dev, sc_qemu_char_dev_read_fn, void *opaque);

#endif
