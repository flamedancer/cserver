/**
run cmd:
make clean && make && ./myserver.out
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>  /* memset */

#include "request.h"
#include "response.h"
#include "tools/poll.h"
#include "config.h"


struct request_buff {
    struct http_request request;
    int data_index;
};

int findEmptyBuffIndex(char * c[50000])
{
    int n = 0;
    while (n < MAXLISTENNUM) {
        if (*(c + n) == 0) {
            return n;
        } else {
            n++;
        }
    }
    return -1;
}


int main() {
    int server_sockfd;
    socklen_t server_len, client_len;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    client_len = sizeof(client_address);

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, MAXLISTENNUM);

    struct PollEvent pollevent;
    initPollEvent(&pollevent);
    // setNonBlock(server_sockfd);
    updateEvents(&pollevent, server_sockfd, Readtrigger, 0, NULL);

    char read_client_buff[MAXLISTENNUM][MAXREQUESTLEN];
    memset(read_client_buff, 0, sizeof(char) * MAXREQUESTLEN * MAXLISTENNUM);

    while(1) {
        int ready_fd_num = doPoll(&pollevent);
        for (int i = 0; i < ready_fd_num; i++) {
            void * eventItem = getIndexEventItem(pollevent.eventItems, i);
            int sock_fd = getFid(eventItem);
            debug_print("this fd is %d \n", sock_fd);
            if (sock_fd <= 2) {
                continue;
            }
            int event_type = getEventType(eventItem);
            debug_print("echo event_type %d \n", event_type);
            if (event_type == Readtrigger) {
                if (sock_fd == server_sockfd) {
                    //Accept a connection
                    int client_sockfd = accept(server_sockfd,
                        (struct sockaddr*)&client_address, (socklen_t*)&client_len);
                    updateEvents(&pollevent, client_sockfd, Readtrigger, 0, NULL);
                } else {
                    if (sock_fd < 0) continue;
                    int this_findEmptyBuffIndex = findEmptyBuffIndex((char**)read_client_buff);

                    if (this_findEmptyBuffIndex < 0)
                        continue;
                    debug_print("use  buff index %d \n", this_findEmptyBuffIndex);

                    int read_len = read(sock_fd, read_client_buff[this_findEmptyBuffIndex], MAXREQUESTLEN);
                    debug_print("%s\n", (char *)read_client_buff[this_findEmptyBuffIndex]);
                    if (read_len <= 0) {
                        close(sock_fd);
                        continue;
                    }
                    // struct http_request request;
                    struct Map headers;
                    initMap(&headers);

                    struct request_buff reBuff;
                    reBuff.request.headers
                        = &headers;
                    reBuff.data_index = this_findEmptyBuffIndex;
                    parse_request(&(reBuff.request), read_client_buff[0] + this_findEmptyBuffIndex);
                    updateEvents(&pollevent, sock_fd, Writetrigger, 1, (void*)(&reBuff));
                }
            } else if (event_type == Writetrigger) {
                struct request_buff* p_buff = (struct request_buff*)(getEventData(eventItem));
                struct http_request* p_request = &(p_buff->request);
                FILE* fp = fdopen(sock_fd, "w+");
                doResponse(p_request, fp);
                fflush(fp);
                fclose(fp);
                // write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0]));

                releaseMap(p_request->headers);

                close(sock_fd);

                memset(read_client_buff[p_buff->data_index], 0, sizeof(char) * MAXREQUESTLEN);
                debug_print("release buff index %d \n", p_buff->data_index);
            }
        }
    }

    releasePollEvent(&pollevent);
}


  
      




