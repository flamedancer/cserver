#ifndef __TASK__
#define __TASK__

#include "config.h"

#define TaskStatus_init 0
#define TaskStatus_doing 1
#define TaskStatus_finish 2

#define TaskType_newClient 0
#define TaskType_readClient 1
#define TaskType_writeClient 2

struct Task {
    int status;
    int fd;
    int type;
    char read_client_buff[MAXREQUESTLEN];
    struct http_response* response;
};

void resetTask();

void doNewClient(struct Task* task);
void doReadClient(struct Task* task);
void doWriteClient(struct Task* task);

struct Task task_poll[MAXLISTENNUM];
void initTaskPoll();
int findFreeTaskIndex();

struct Task* pushNewTask(int fd, int type, struct http_response* response);

#endif
