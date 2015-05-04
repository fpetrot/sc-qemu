#ifndef _RABBITS_TARGET_TARGET_H
#define _RABBITS_TARGET_TARGET_H

#include "../qemu_context.h"

void sc_qemu_machine_init_target(qemu_context *ctx, const char *model);
void sc_qemu_target_qdev_create(sc_qemu_qdev *ret, sc_qemu_qdev_e devid, va_list ap);

#endif
