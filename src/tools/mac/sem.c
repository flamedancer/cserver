#include "../sem.h"

int new_sem(sem_t** p_sem, const char* sem_name)
{
    // todo ！！莫名其妙! 去掉下面这句话会 创建不成功！！ 有毒吧！
    sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR, 0, 0);

    *p_sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR, 0, 0);
    if (*p_sem == SEM_FAILED || *p_sem == NULL) {
        perror("new_sem failed");
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
