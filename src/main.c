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
    int server_sockfd;
    socklen_t server_len;

    struct sockaddr_in server_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr*)&server_address, server_len);

    listen(server_sockfd, MAXLISTENNUM);

    initPollEvent(&pollevent);
    // setNonBlock(server_sockfd);

    updateEvents(&pollevent, server_sockfd, Readtrigger, 0, &server_sockfd);

    initTaskPoll();
    int c = 0;
    int d = 0;
    while (1) {
        int ready_fd_num = doPoll(&pollevent);
        for (int i = 0; i < ready_fd_num; i++) {
            void* eventItem = getIndexEventItem(pollevent.eventItems, i);
            int sock_fd = getFid(eventItem);
            int event_type = getEventType(eventItem);
            debug_print("this fd is %d \n", sock_fd);
            debug_print("echo event_type %d \n", event_type);
            if (sock_fd <= 2) {
                continue;
            }
            if (event_type == Readtrigger) {
                if (sock_fd == server_sockfd) {
                    struct Task* task = pushNewTask(sock_fd, TaskType_newClient, NULL);
                    doNewClient(task);
                    resetTask(task);
                } else {
                    struct Task* task = pushNewTask(sock_fd, TaskType_readClient, NULL);
                    doReadClient(task);
                    resetTask(task);
                    d++;
                    printf("has start %d \n", d);
                }
            } else if (event_type == Writetrigger) {
                struct http_response* response = getEventData(eventItem);
                struct Task* task = pushNewTask(sock_fd, TaskType_writeClient, response);
                doWriteClient(task);
                resetTask(task);
                c++;
                printf("has handle %d \n", c);
            }
        }
    }

    releasePollEvent(&pollevent);
}
