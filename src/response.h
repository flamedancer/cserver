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
    int body_size;
};

void initHttpResponse(struct http_response * response);

void doResponse(
    struct http_request * request, 
    FILE * stream
);
void setResponseMsg(struct http_response *response, const char * msg);

void outputToFile(
    struct http_response * response,
    FILE * stream
);

void responeFileContent(
    char * filePath, 
    struct http_response * response
);

void show_dir_content(struct http_response *response);

void doCgi(
    char *filePath,
    struct http_response *response);