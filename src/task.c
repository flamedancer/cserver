#include "task.h"
#include "main.h"
#include "request.h"
#include "response.h"
#include "tools/poll.h"
#include <arpa/inet.h>
#include <errno.h> /* errno */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <time.h>
#include <unistd.h> /* read close fd */

const struct timespec spec = { 0, 100 };

void resetTask(struct Task* task)
{
    task->fd = -1;
    task->response = NULL;
    task->status = TaskStatus_finish;
    task->type = TaskType_newClient;
    memset(task->read_client_buff, 0, sizeof(char) * MAXREQUESTLEN);
}

void initTaskPoll()
{
    for (int i = 0; i < MAXLISTENNUM; i++) {
        resetTask(&(task_poll[i]));
    }
}

struct Task* selectTask(int status)
{
    for (int i = 0; i < MAXLISTENNUM; i++) {
        printf("index: %d fd: %d taskType: %d status: %d %d, \n", i, task_poll[i].fd, task_poll[i].type, task_poll[i].status, status);
        if (task_poll[i].status == status) {
            return &task_poll[i];
        }
    }
    return NULL;
}

/* 获取并设置任务状态 原子操作  */
struct Task* getAndSetStatusTask(int selectStatus, int newStatus)
{
    debug_print("try get task %d and set to %d\n", selectStatus, newStatus);

    pthread_mutex_lock(&work_mutex);
    struct Task* task = selectTask(selectStatus);

    while (task == NULL) {
        nanosleep(&spec, NULL);
        task = selectTask(selectStatus);
        // perror("Cannot pushNewTask");
        // exit(EXIT_FAILURE);
    }
    task->status = newStatus;
    pthread_mutex_unlock(&work_mutex);

    return task;
}

struct Task* pushNewTask(int fd, int type, struct http_response* response)
{
    struct Task* task = getAndSetStatusTask(TaskStatus_finish, TaskStatus_init);
    task->fd = fd;
    task->type = type;
    task->response = response;
    sem_post(bin_sem);
    return task;
}

void doNewClient(struct Task* task)
{
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int server_sockfd = task->fd;
    resetTask(task);
    //Accept a connection
    while (1) {
        int client_sockfd
            = accept(server_sockfd,
                // (struct sockaddr*)&client_address, (socklen_t*)&client_len);
                NULL, NULL);
        if (client_sockfd == -1) {
            if (errno == EWOULDBLOCK) {
                break;
            } else {
                printf("server fd is  %d \n", server_sockfd);
                perror("error when client_sockfd accept");
                return;
                // exit(EXIT_FAILURE);
            }
        }
        printf("has client start %d \n", client_sockfd);
        
        setNonBlock(client_sockfd);

        updateEvents(&pollevent, client_sockfd, Readtrigger, TriggerPolicy_ONESHOT, 0, NULL);
        // break;
    }
}

void doReadClient(struct Task* task)
{
    int sock_fd = task->fd;
    int read_len = read(sock_fd, task->read_client_buff, MAXREQUESTLEN);
    debug_print("fd %d read buff %s\n", sock_fd, (char*)task->read_client_buff);
    if (read_len <= 0) {
        close(sock_fd);
        resetTask(task);
        return;
    }
    struct http_request request;
    struct Map headers;
    initMap(&headers);
    request.headers = &headers;
    parse_request(&request, task->read_client_buff);
    struct http_response* response = doResponse(&request);

    releaseMap(request.headers);

    resetTask(task);
    updateEvents(&pollevent, sock_fd, Writetrigger, TriggerPolicy_ONESHOT, 1, (void*)response);
}

void doWriteClient(struct Task* task)
{

    int sock_fd = task->fd;

    FILE* fp = fdopen(sock_fd, "w+");
    outputToFile(task->response, fp);
    releaseResponse(task->response);

    fclose(fp);
    // write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0]));
    close(sock_fd);
    printf("has client close %d \n", sock_fd);
    resetTask(task);
}

void* doTask()
{
    while (1) {
        sem_wait(bin_sem);
        struct Task* task = getAndSetStatusTask(TaskStatus_init, TaskStatus_doing);
        switch (task->type) {
        case TaskType_newClient:
            debug_print("doNewClient\n");
            doNewClient(task);

            break;
        case TaskType_readClient:
            debug_print("doReadClient\n");
            doReadClient(task);

            break;
        case TaskType_writeClient:
            debug_print("writeClient\n");
            doWriteClient(task);

            break;
        default:
            perror("no that Task Type");
            exit(EXIT_FAILURE);
        }
        debug_print("finish task\n");
    }
}
