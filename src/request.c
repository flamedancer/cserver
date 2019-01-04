/* request.c 
*/
#include "request.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_request(
    struct http_request* request,
    char* http_data,
    int http_data_len)
{
    char* start = http_data;
    char* end = start + http_data_len;
    // 解析第一行
    char* method = start;
    char* url = NULL;
    char* version = NULL;
    for (; *start && *start != '\r' && start < end; start++) {
        //  method url version 是由 空格 分割的
        if (*start == ' ') {
            if (url == NULL) {
                url = start + 1;
            } else {
                version = start + 1;
            }
            *start = '\0';
        }
    }
    if (url == NULL || version == NULL) {
        return;
    }
    *start = '\0'; // \r -> \0
    start++; // skip \n
    request->method = method;
    request->url = url;
    request->version = version;
    request->body = NULL;
    /* 第一行解析结束 */
    /* 第二行开始为 header  解析hedaer*/
    start++; // 第二行开始
    char* line = start;
    char* key;
    char* value;
    while (*line != '\r' && *line != '\0') {
        char* key;
        char* value;
        while (*(start++) != ':')
            ;
        *(start - 1) = '\0';
        key = line;
        value = start;
        // todo 超过 MAXREQUESTLEN 的 判断
        while (start++, *start != '\0' && *start != '\r')
            ;
        *start++ = '\0'; // \r -> \0
        start++; // skip \n

        // printf("key is %s \n", key);
        // printf("value is %s \n", value);
        line = start;

        struct Item* item = newItem(key, value);
        mapPush(request->headers, item);
    }
    /* 如果最后一行不是空行  说明有body数据 */
    if (*line == '\r') {
        char* len_str = mapGet(request->headers, "Content-Length");
        if (len_str != NULL) {
            int len = atoi(len_str);
            // 跳过 两个 \n
            line = line + 2;
            *(line + len) = '\0';
            request->body = line;
        }
    }

    /*  打印 request 信息 */
    debug_print("---------------------------\n");
    debug_print("method is: %s \n", request->method);
    debug_print("url is: %s \n", request->url);
    debug_print("http version is: %s \n", request->version);
    debug_print("the headers are :\n");
    mapPrint(request->headers);
    debug_print("body is %s \n", request->body);
    debug_print("---------------------------\n");
}

void releaseRequest(struct http_request* request)
{
}
