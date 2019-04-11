#include "../sem.h"

int new_sem(sem_t* sem, const char* sem_name)
{
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
    return sem_destroy(sem);
}