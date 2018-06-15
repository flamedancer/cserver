/* test/requestTest.c
*/
#include <stdio.h>
#include "../request.h" 


int main() {
    struct http_request request;
    char data[] = "POST / HTTP/1.1\n";
    parse_request(&request, data); 
    printf("method is %s; url is %s; version is %s \n", request.method, request.url, request.version);
}
