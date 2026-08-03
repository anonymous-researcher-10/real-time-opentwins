#include "core_binding.h"

void pin_thread_to_core(int core_id, int rt_priority) {

    // Set CPU affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();

    // Establecer afinidad de CPU
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        printf("[ERROR] Thread could not be linked to core %d\n", core_id);
        return;
    }

    // Establecer prioridad en tiempo real
    struct sched_param param;
    param.sched_priority = rt_priority; // Prioridad entre 1 y 99
    if (pthread_setschedparam(current_thread, SCHED_FIFO, &param) != 0) {
        printf("[ERROR] Failed to assign RT priority to core %d. Did you use sudo?\n", core_id);
    } else {
        printf("[SYSTEM] Thread configured successfully on Core %d with priority %d\n", core_id, rt_priority);
    }

    printf("[SYSTEM] Thread %lu assigned to core %d with priority %d\n", current_thread, core_id, rt_priority);
}