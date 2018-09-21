/* response.h
*/
#include <stdio.h>
#include "tools/utils.h"
#include "request.h"

struct http_response {
    char * version;
    char * code;   // 状态返回码
    char * desc;   // 返回描述
    struct Map * headers; 
    char * body; 
};

void initHttpResponse(struct http_response * response);

void doResponse(
    struct http_request * request, 
    FILE * stream
);

void outputToFile(
    struct http_response * response,
    FILE * stream,
    int body_len
);
