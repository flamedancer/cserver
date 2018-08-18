/* test/responseTest.c
test cmd:
gcc  ../request.h ../request.c ../response.h ../response.c ../tools/utils.h ../tools/utils.c  responseTest.c && ./a.out
*/
#include <stdio.h>
#include "../request.h" 
#include "../response.h" 


int main() {
    struct http_request request;
    char data[] = "POST / HTTP/1.1\r\nContent-Length: 3\r\n\r\n111";
    struct Map headers;
    request.headers = &headers;
    parse_request(&request, data); 
    doResponse(&request, stdout);
}
