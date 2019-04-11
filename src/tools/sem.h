#ifndef __SEM__
#define __SEM__

#include <semaphore.h>
#include <stdio.h> /* fprintf  NULL */

int new_sem(sem_t** sem, const char* sem_name);
int v_sem(sem_t* sem);
int p_sem(sem_t* sem);
int remove_sem(sem_t* sem, const char* sem_name);

#endif