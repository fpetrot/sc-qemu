#include "sysemu/char.h"
#include "sysemu/sysemu.h"

#include "sc_qemu_char_dev.h"

struct sc_qemu_char_dev {
    qemu_context *ctx;
    struct CharDriverState *s;
};

sc_qemu_char_dev* sc_qemu_char_dev_create(qemu_context *ctx)
{
    sc_qemu_char_dev *ret = g_malloc0(sizeof(sc_qemu_char_dev));

    ret->ctx = ctx;
    ret->s = serial_hds[0]; /* XXX */

    return ret;
}

int sc_qemu_char_dev_write(sc_qemu_char_dev *dev, const uint8_t *data, int len)
{
    return qemu_chr_fe_write(dev->s, data, len);
}

static int sc_qemu_char_can_read(void* opaque)
{
    return 1;
}

void sc_qemu_char_dev_register_read(sc_qemu_char_dev *dev, sc_qemu_char_dev_read_fn fn, void *opaque)
{
    qemu_chr_add_handlers(dev->s, sc_qemu_char_can_read, fn, NULL, opaque);
}
