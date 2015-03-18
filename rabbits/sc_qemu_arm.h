#ifndef _RABBITS_SC_QEMU_ARM_H
#define _RABBITS_SC_QEMU_ARM_H

/* ARM irq lines index 
 * I don't want to include to much QEMU header files here so the contants are
 * hardcoded. */
typedef enum sc_qemu_arm_irq_e {
    SC_QEMU_ARM_IRQ_IRQ = 0 /* ARM_CPU_IRQ */,
    SC_QEMU_ARM_IRQ_FIQ = 1 /* ARM_CPU_FIQ */,
} sc_qemu_arm_irq_e;

#endif
