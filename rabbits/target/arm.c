#include "hw/sysbus.h"
#include "sysemu/sysemu.h"

#include "target.h"
#include "../sc_qemu.h"
#include "../sc_qdev_priv.h"

void sc_qemu_machine_init_target(qemu_context *ctx, const char *model)
{
    ARMCPU *cpu;
    char const *m = model;
    int i;

    if(model == NULL) {
        m = "arm1176";
    }

    ctx->cpus = g_malloc0(sizeof(CPUState*) * smp_cpus);

    for(i = 0; i < smp_cpus; i++) {
        cpu = cpu_arm_init(m);
        if (!cpu) {
            fprintf(stderr, "Unable to find CPU definition\n");
            exit(1);
        }

        ctx->cpus[i] = CPU(cpu);
    }
}

void sc_qemu_target_qdev_create(sc_qemu_qdev *ret, sc_qemu_qdev_e devid, va_list ap)
{
    uint32_t u;
    int i;

    switch(devid) {
    case SC_QDEV_A15PRIV:
        u = va_arg(ap, uint32_t); /* num-irq property */
        ret->dev = qdev_create(NULL, "a15mpcore_priv");

        qdev_prop_set_uint32(ret->dev, "num-cpu", ret->ctx->num_cpu);
        qdev_prop_set_uint32(ret->dev, "num-irq", u);

        qdev_init_nofail(ret->dev);

        for (i = 0; i < ret->ctx->num_cpu; i++) {
            DeviceState *cpudev = DEVICE(ret->ctx->cpus[i]);
            sysbus_connect_irq(SYS_BUS_DEVICE(ret->dev), i, qdev_get_gpio_in(cpudev, ARM_CPU_IRQ));
        }

        break;

    default:
        break;
    }
}
