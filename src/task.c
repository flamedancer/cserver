#include "task.h"
#include "main.h"
#include "request.h"
#include "response.h"
#include "tools/poll.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h> /* memset */
#include <unistd.h> /* read close fd */

void resetTask(struct Task* task)
{
    task->fd = 0;
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

int findFreeTaskIndex()
{
    for (int i=0; i < MAXLISTENNUM; i++) {
        if (task_poll[i].status == TaskStatus_finish) {
            return i;
        }
    }
    return -1;
}

struct Task* pushNewTask(int fd, int type, struct http_response* response)
{
    int task_index = findFreeTaskIndex();
    if (task_index < 0) {
        err(1, "Cannot  pushNewTask");
        return -1;
    }
    struct Task* task = &task_poll[task_index];
    task->fd = fd;
    task->type = type;
    task->status = TaskStatus_init;
    task->response = response;
    return task;
}

void doNewClient(struct Task* task)
{
    struct sockaddr_in client_address;
    int client_len = sizeof(client_address);
    int server_sockfd = task->fd;
    //Accept a connection
    int client_sockfd
        = accept(server_sockfd,
            (struct sockaddr*)&client_address, (socklen_t*)&client_len);
    updateEvents(&pollevent, client_sockfd, Readtrigger, 0, &client_sockfd);
}

void doReadClient(struct Task* task)
{
    int sock_fd = task->fd;
    int read_len = read(sock_fd, task->read_client_buff, MAXREQUESTLEN);
    debug_print("%s\n", (char*)task->read_client_buff);
    if (read_len <= 0) {
        close(sock_fd);
        return;
    }
    struct http_request request;
    struct Map headers;
    initMap(&headers);
    request.headers = &headers;
    parse_request(&request, task->read_client_buff);
    struct http_response* response = doResponse(&request);
    
    releaseMap(request.headers);
    updateEvents(&pollevent, sock_fd, Writetrigger, 1, (void*)response);
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
}
