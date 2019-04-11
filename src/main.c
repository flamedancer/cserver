/**
run cmd:
make clean && make && ./myserver.out
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "main.h"
#include "request.h"
#include "response.h"
#include "task.h"
#include "tools/poll.h"


int main()
{
    // todo 1. 执行流程图     2. 为什么noblock  3. 事件触发策略
    prepareWork();

    while (1) {
        int ready_fd_num = doPoll(&pollevent);
        for (int i = 0; i < ready_fd_num; i++) {
            printf("i is %d \n", i);
            void* eventItem = getIndexEventItem(pollevent.eventItems, i);
            
            // int sock_fd = getFid(eventItem);
            int sock_fd = server_sockfd;
            printf("y \n");
            int event_type = getEventType(eventItem);
            debug_print("this fd is %d \n", sock_fd);
            debug_print("echo event_type %d \n", event_type);
            if (sock_fd <= 2) {
                continue;
            }
            // fd
            // 设置任务 状态  为  可以io读写
            setStatusInitByFd(sock_fd);
            v_sem(&bin_sem);
            // sleep(5);
        }
    }

    cleanWork();
    printf("whats happen !");
    exit(EXIT_SUCCESS);
}

void initServerSocket()
{

    socklen_t server_len;

    struct sockaddr_in server_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (DEBUG) {
        int ov = 1;
        setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&ov, sizeof ov);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr*)&server_address, server_len);
    setNonBlock(server_sockfd);
    listen(server_sockfd, MAXLISTENNUM);
}

void prepareWork()
{
    initServerSocket();
    initPollEvent(&pollevent);
    initTaskPoll();

    printf("x step \n");
    
    // 监听事件 : server_sockfd 可read
    updateEvents(&pollevent, server_sockfd, Readtrigger, TriggerPolicy_CLEAR, 0, 0);

    printf("xx step \n");

    int res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    printf("xxx step \n");

    int err = new_sem(&bin_sem, SEM_NAME);
    printf("xxx1 step \n");
    if (err == -1) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }

     printf("xxxx step \n");

    for (int i = 0; i < MAXLISTENNUM; i++) {
        res = pthread_create(&thread_pool[i], NULL, doTask, NULL);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    printf("xxxxx step \n");

}

void cleanWork()
{
    int res;
    void* thread_result;
    printf("\nWaiting for thread to finish...\n");
    for (int i = 0; i < MAXLISTENNUM; i++) {
        res = pthread_join(thread_pool[i], &thread_result);
        if (res != 0) {
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
        printf("Thread joined ,index is %d \n", i);
    }
    releasePollEvent(&pollevent);
    pthread_mutex_destroy(&work_mutex);
    remove_sem(&bin_sem, SEM_NAME);
    close(server_sockfd);
}
