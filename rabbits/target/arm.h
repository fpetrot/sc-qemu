#ifndef _RABBITS_TARGET_ARM_H
#define _RABBITS_TARGET_ARM_H

/* ARM irq lines index 
 * I don't want to include to much QEMU header files here so the contants are
 * hardcoded. */
typedef enum sc_qemu_arm_irq_e {
    SC_QEMU_ARM_IRQ_IRQ = 0 /* ARM_CPU_IRQ */,
    SC_QEMU_ARM_IRQ_FIQ = 1 /* ARM_CPU_FIQ */,
} sc_qemu_arm_irq_e;


typedef enum sc_qemu_qdev_arm_e {
    /* Variadic initialisation :
     *  - number of interrupts, uint32_t
     * MMIO Mapping :
     *  0: GIC  base address
     */
    SC_QDEV_ARM_A15PRIV = SC_QDEV_LAST,
} sc_qemu_qdev_arm_e;

typedef enum sc_qdev_a15priv_mmio_e {
    SC_QDEV_A15PRIV_MMIO_GIC = 0,
} sc_qdev_a15priv_mmio_e;

#endif
