#include "../sem.h"
#include <stdlib.h>

int new_sem(sem_t** p_sem, const char* sem_name)
{
    sem_t* sem = (sem_t*)malloc(sizeof(sem_t));
    *p_sem = sem;
    if (sem == NULL) {
        perror("new_sem failed");
        exit(EXIT_FAILURE);
    }
    memset(sem, 0, sizeof(sem_t));
    return sem_init(sem, 0, 0);
}

int v_sem(sem_t* sem)
{
    return sem_post(sem);
}

int p_sem(sem_t* sem)
{
    return sem_wait(sem);
}

int remove_sem(sem_t* sem, const char* sem_name)
{
    int error = sem_destroy(sem);
    if (error == -1) {
        return error;
    }
    free(sem);
    return error;
}