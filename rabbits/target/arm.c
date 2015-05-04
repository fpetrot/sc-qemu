#include "../qemu_context.h"
#include "target.h"

void sc_qemu_machine_init_target(qemu_context *ctx, const char *model)
{
    ARMCPU *cpu;
    char const *m = model;
    int i;

    if(model == NULL) {
        m = "arm1176";
    }

    ctx->cpus = g_malloc0(sizeof(ARMCPU*) * smp_cpus);

    for(i = 0; i < smp_cpus; i++) {
        cpu = cpu_arm_init(m);
        if (!cpu) {
            fprintf(stderr, "Unable to find CPU definition\n");
            exit(1);
        }

        ctx->cpus[i] = CPU(cpu);
    }
}
