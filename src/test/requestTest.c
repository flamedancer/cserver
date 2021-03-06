/* test/requestTest.c
test cmd: gcc  ../request.h ../request.c ../tools/utils.h ../tools/utils.c  requestTest.c && ./a.out
*/
#include <stdio.h>
#include "../request.h" 


int main() {
    struct http_request request;
    char data[] = "POST / HTTP/1.1\r\nContent-Length: 3\r\n\r\n111";
    struct Map headers;
    request.headers = &headers;
    parse_request(&request, data); 
    printf("method is %s; url is %s; version is %s \n", request.method, request.url, request.version);
}
