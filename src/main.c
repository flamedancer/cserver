#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "request.h"

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
        char ch[50000];
        char send_str[] = "hello world !\n";
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
        (struct sockaddr *)&client_address, &client_len);

        read(client_sockfd, &ch, 50000);
        printf("%s\n", ch);
        struct http_request request;
        parse_request(&request, ch); 
        printf("method is %s; url is %s; version is %s \n", request.method, request.url, request.version);
        
        write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0])); 
        close(client_sockfd);
    }
}


  
      




