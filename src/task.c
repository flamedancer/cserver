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

const struct timespec spec = { 0, 1000 };
const int max_try_cnt = 30;

void print_task(struct Task* task)
{
    debug_print("index %d fd %d status %d type %d \n", task->index, task->fd, task->status, task->type);
}

void setTask(struct Task* task, int fd, int status, int type)
{
    print_task(task);
    task->fd = fd;
    task->status = status;
    task->type = type;
    print_task(task);
}

void resetTask(struct Task* task)
{
    if (task->fd != server_sockfd) {
        close(task->fd);
        printf("has client close %d \n", task->fd);
    }
    task->fd = server_sockfd;
    task->status = TaskStatus_ready;
    task->type = TaskType_newClient;
    memset(read_client_buff[task->index], 0, sizeof(char) * MAXREQUESTLEN);
    releaseResponse(response_cache[task->index]);
    response_cache[task->index] = NULL;
    printf("reset task index %d \n", task->index);
    print_task(task);
}

void initTaskPoll()
{
    for (int i = 0; i < MAXLISTENNUM; i++) {
        task_poll[i].index = i;
        resetTask(&task_poll[i]);
    }
}

struct Task* selectTaskByStatus(int status)
{
    debug_print("find status is %d and type is 0 \n", status);
    for (int i = 0; i < MAXLISTENNUM; i++) {
        print_task(&task_poll[i]);
        if (task_poll[i].status == status && task_poll[i].type == TaskType_newClient) {
            printf("get suit task index is %d \n", i);
            return &task_poll[i];
        }
    }
    return NULL;
}

/* 获取并设置任务状态 原子操作  */
struct Task* getAndSetStatusTask(int selectStatus, int newStatus)
{
    debug_print("try get task status %d and set to %d\n", selectStatus, newStatus);

    pthread_mutex_lock(&work_mutex);
    struct Task* task = selectTaskByStatus(selectStatus);
    if (task != NULL) {
        task->status = newStatus;
    }
    pthread_mutex_unlock(&work_mutex);

    return task;
}

struct Task* selectTaskByFd(int fd)
{
    debug_print("find fd is %d and status is 0 \n", fd);

    for (int i = 0; i < MAXLISTENNUM; i++) {
        print_task(&task_poll[i]);
        if (task_poll[i].fd == fd && task_poll[i].status == TaskStatus_ready) {
            return &task_poll[i];
        }
    }
    return NULL;
}

struct Task* setStatusInitByFd(int fd)
{
    debug_print("try get ready fd %d and set init\n", fd);

    pthread_mutex_lock(&work_mutex);
    struct Task* task = selectTaskByFd(fd);
    int retry_cnt = 0;
    while (task == NULL && retry_cnt < max_try_cnt) {
        sem_post(bin_sem);
        retry_cnt++;
        nanosleep(&spec, NULL);
        task = selectTaskByFd(fd);
    }
    if (task != NULL) {
        task->status = TaskStatus_init;
        print_task(task);
    }
    pthread_mutex_unlock(&work_mutex);
    return task;
}

void doNewClient(struct Task* task)
{
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int client_num = 0;
    //Accept a connection

    while (1) {
        client_num++;
        printf("to get cient: index is %d \n", task->index);

        setNonBlock(server_sockfd);

        int client_sockfd
            = accept(server_sockfd,
                (struct sockaddr*)&client_address, (socklen_t*)&client_len);
        printf("to get cient ok: index is %d , fd is %d \n", task->index, client_sockfd);
        if (client_sockfd == -1) {
            if (client_num == 1) {
                resetTask(task);
            }
            if (errno == EWOULDBLOCK) {
                break;
            } else {
                printf("server fd is  %d \n", server_sockfd);
                perror("error when client_sockfd accept");
                // return;
                exit(EXIT_FAILURE);
            }
        }
        debug_print("index %d has client start %d \n", task->index, client_sockfd);
        debug_print("index %d client_num is %d \n", task->index, client_sockfd);
        setNonBlock(client_sockfd);
        if (client_num > 1) {
            
            task = setStatusInitByFd(server_sockfd);
            if (task != NULL) {
                setTask(task, client_sockfd, TaskStatus_ready, TaskType_readClient);
                // updateEvents(&pollevent, client_sockfd, Readtrigger, TriggerPolicy_ONESHOT, 0, NULL);
                // sem_post(bin_sem);
            }else {
                return;
            }
        } else {
            setTask(task, client_sockfd, TaskStatus_ready, TaskType_readClient);
            // updateEvents(&pollevent, client_sockfd, Readtrigger, TriggerPolicy_ONESHOT, 0, NULL);
        }
        if (doReadClient(task)) {
            doWriteClient(task);
        }
        resetTask(task);
    }
}

int doReadClient(struct Task* task)
{
    int sock_fd = task->fd;
    int read_len;
    int retry_cnt = 0;
    char* read_buff = read_client_buff[task->index];
    // memset(read_buff, 0, MAXREQUESTLEN);

    do {
        read_len = read(sock_fd, read_buff, MAXREQUESTLEN);
        if (read_len > 0) {
            break;
        };
        nanosleep(&spec, NULL);
        retry_cnt++;
    } while (read_len <= 0 && retry_cnt < max_try_cnt - 1);
    if (read_len <= 0) {
        printf(" client read timeout, abandon \n");
        resetTask(task);

        // 补丁  不知道怎么产生的read事件  忽略 并且补充 一个 sem
        // sem_post(bin_sem);
        return 0;
    }
    debug_print("the request of %d is \n%s \n", sock_fd, read_buff);

    struct http_request request;
    struct Map headers;
    initMap(&headers);
    request.headers = &headers;
    debug_print("111  read len %d  \n", read_len);
    parse_request(&request, read_buff, read_len);
    if (request.url == NULL) {
        printf(" client data cannot parse to request, abandon \n");
        resetTask(task);
        return 0;
    }
    debug_print("222\n");

    struct http_response* response = doResponse(&request);
    response_cache[task->index] = response;

    releaseMap(&headers);
    setTask(task, sock_fd, TaskStatus_ready, TaskType_writeClient);
    // updateEvents(&pollevent, sock_fd, Writetrigger, TriggerPolicy_ONESHOT, 1, NULL);
    return 1;
}

void doWriteClient(struct Task* task)
{

    int sock_fd = task->fd;

    FILE* fp = fdopen(sock_fd, "w+");
    outputToFile(response_cache[task->index], fp);
    fclose(fp);

    // write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0]));
    // resetTask(task);
}

void* doTask()
{
    struct Task* task;
    while (1) {
        int error = p_sem(bin_sem);
        if (error != 0) {
            perror("sem wait error");
        }
        task = NULL;
        while ((task = getAndSetStatusTask(TaskStatus_init, TaskStatus_doing)) != NULL)
        {
            printf("get ready task index %d \n", task->index);
            switch (task->type) {
            case TaskType_newClient:
                debug_print("doNewClient\n");
                doNewClient(task);
                debug_print("New OK\n");
                break;
            // case TaskType_readClient:
            //     debug_print("doReadClient\n");
            //     doReadClient(task);
            //     debug_print("do read OK\n");

            //     break;
            // case TaskType_writeClient:
            //     debug_print("writeClient\n");
            //     doWriteClient(task);
            //     debug_print("write ok finish task\n");
            //     break;
            default:
                ;
                // perror("no that Task Type");
                // printf("type is %d \n", task->type);
                // exit(EXIT_FAILURE);
            }
            // break;
        }
    }
}
