/**
run cmd:
gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out

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
#define MAXREQUESTLEN 50000

int main() {
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);
    while(1) {

        char ch[MAXREQUESTLEN];
        memset(ch, 0, sizeof(char) * MAXREQUESTLEN);
        // char send_str[] = "hello world !\n";
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
        (struct sockaddr *)&client_address, &client_len);

        read(client_sockfd, &ch, MAXREQUESTLEN);
        printf("%s\n", ch);
        struct http_request request;
        struct Map headers;
        request.headers = &headers;
        initMap(request.headers);

        parse_request(&request, ch); 
        FILE* fp = fdopen(client_sockfd, "w+");
        doResponse(&request, fp);
        fflush(fp);
        fclose(fp);
        // write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0])); 

        // clean
        releaseMap(request.headers);
    }
}


  
      




