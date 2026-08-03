#define CORE_BINDING_H

#define _GNU_SOURCE // Obligatorio antes de incluir sched.h o pthread.h
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

void pin_thread_to_core(int core_id, int rt_priority);
