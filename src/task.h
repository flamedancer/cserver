#ifndef __TASK__
#define __TASK__

#include "config.h"

#define TaskStatus_ready 0 //  业务逻辑任务准备好中 fd没有准备好
#define TaskStatus_init 1 //   业务逻辑任务准备  fd也可以读/写了
#define TaskStatus_doing 2
#define TaskStatus_finish 3

#define TaskType_newClient 0
#define TaskType_readClient 1
#define TaskType_writeClient 2

struct Task {
    int index;
    int status;
    int fd;
    int type;
};

void setTask(struct Task* task, int fd, int status, int type);
void resetTask();

void doNewClient(struct Task* task);
int doReadClient(struct Task* task);
void doWriteClient(struct Task* task);

struct Task task_poll[MAXLISTENNUM];
char read_client_buff[MAXLISTENNUM][MAXREQUESTLEN];
struct http_response* response_cache[MAXLISTENNUM];

void initTaskPoll();
struct Task* selectTaskByStatus(int status);
struct Task* getAndSetStatusTask(int selectStatus, int newStatus);

struct Task* selectTaskByFd(int fd);
struct Task* setStatusInitByFd(int fd);

void* doTask();

#endif
