/* request.c 
*/
#include "request.h"

void parse_request(
    struct http_request * request,
    char * http_data) {
    char * start = http_data;
    // 解析第一行
    char * method = start;
    char * url = 0;
    char * version = 0;
    for(;*start && *start != '\n'; start++) {
        //  method url version 是由 空格 分割的
        if(*start == ' ') {
            if(url == 0) {
                url = start + 1;
            } else {
                version = start + 1;
            }
            *start = '\0';
        }
    }
    *start = '\0';
    request->method = method;
    request->url = url;
    request->version = version;
}








