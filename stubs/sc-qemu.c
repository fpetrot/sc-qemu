#include "qemu/osdep.h"
#include "qemu/compiler.h"

void QEMU_NORETURN __real_abort(void);

void QEMU_NORETURN __wrap_abort(void);
void QEMU_NORETURN __wrap_abort(void)
{
    __real_abort();
}

void QEMU_NORETURN __real_exit(int status);

void QEMU_NORETURN __wrap_exit(int status);
void QEMU_NORETURN __wrap_exit(int status)
{
    __real_exit(status);
}

void sc_qemu_do_register_ctor(void (*fn)(void))
{
}
