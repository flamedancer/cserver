#include "../sem.h"

int new_sem(sem_t* sem, const char* sem_name)
{
    sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR, 0, 0);
    if (sem == SEM_FAILED) {
        return -1;
    }
    return 0;
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
    sem_close(sem);
    return sem_unlink(sem_name);
}
