#ifndef __MAIN__
#define __MAIN__
#include "tools/sem.h"
#include <pthread.h>

int server_sockfd;

struct PollEvent pollevent;
pthread_mutex_t work_mutex;
sem_t bin_sem;
#define SEM_NAME "some_sem"
pthread_t thread_pool[MAXLISTENNUM];

void initServerSocket();
void prepareWork();
void cleanWork();

#endif
