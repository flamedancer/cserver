/* response.c
*/
#include "config.h"
#include "response.h"
#include "config.h"
#include "request.h"
#include "tools/utils.h"
#include <dirent.h> /* DIR opendir DT_DIR */
#include <stdio.h> /* fprintf  NULL */
#include <stdlib.h> /* malloc, free, rand */
#include <string.h> /* strlen */
#include <unistd.h> /* access */

// #define home_url "/";
const char * home_url = "/";
const char * static_url = "/static/";
const char * action_url = "/action/";

const char* errorMsg = "<html><meta charset='utf-8'>"
                       "<a href='/'> >_< 看来你迷路了 </a>"
                       "<p> cserver error: File Not Found </p>"
                       "your path is %s"
                       "</html>";

void initHttpResponse(struct http_response *response) {
    response->version = NULL;
    response->code = NULL;
    response->desc = NULL;
    response->headers = NULL;
    response->body = NULL;
}


void doResponse(struct http_request * request, FILE * stream) {
    struct http_response responseInstance;
    struct http_response * response = &responseInstance; 
    initHttpResponse(response);
    struct Map map_instance;
    initMap(&map_instance);
    response->headers = &map_instance;

    response->version = "HTTP/1.1";
    response->code = "200";
    response->desc = "OK";


    if (strncmp(static_url, request->url, strlen(static_url)) == 0) {
        responeFileContent(request->url + 1, response);
    } else if (strncmp(action_url, request->url, strlen(action_url)) == 0) {
        doCgi(request->url + 1, response);
    } else if (strncmp(home_url, request->url, strlen(request->url)) == 0) {
        show_dir_content(response);
    } else {
        setResponseMsg(response, errorMsg, request->url);
    }

    char content_len[25];
    sprintf(content_len, "%d", response->body_size);
    struct Item * item = newItem(
        "Content-Length",
        content_len
    );
    mapPush(response->headers, item);
    
    outputToFile(response, stream);

    // clean
    free(response->body);   // If ptr is NULL, no operation is performed.
    response->body = NULL;
    releaseMap(response->headers);
}


void outputToFile(struct http_response * response, FILE * stream) {
    // output version code desc
    int r = fprintf(stream, "%s %s %s \r\n",
        response->version,
        response->code,
        response->desc
    );
    // output headers
    struct Map* map = response->headers;
    struct List* list;
    struct Item* item;
    int print_item_cnt = 0;
    for(int i=0; i<map->table_len; i++) {
        list = map->table[i];
        if(list == NULL) {
           continue;
        }
        item = list->start;
        while(item != NULL) {
            fprintf(stream, "%s: %s\r\n",
                item->key,
                (char *)(item->value)
            );
            item = item->next;
        }
    }
    // output body
    if(response->body_size > 0 && response->body != NULL) {
        fprintf(stream, "\r\n");
        fwrite(response->body, response->body_size, 1, stream);
        // printf(" ------  this resp is : \n%s \n  ---------\n", response->body);
    }
}

void setResponseMsg(struct http_response* response, const char* msg, const char* url)
{
    // meta 优先级比 header 高， 所以没必要 设置Content-Type: text/html; charset=utf-8
    response->body_size = (int)strlen(msg) + (int)strlen(url);
    response->body = (char *)malloc((response->body_size));
    sprintf(response->body, msg, url);
    // strcpy(response->body, msg);
}


void responeFileContent(char * filePath, struct http_response * response) {
    char * error_file = "static/404.html";
    FILE * fileptr;
    fileptr = fopen(filePath, "rb");
    if (NULL == fileptr) {
        fileptr = fopen(error_file, "rb");
    }
    fseek(fileptr, 0, SEEK_END);
    response->body_size = ftell(fileptr);
    rewind(fileptr);
    response->body = (char *)malloc((response->body_size));
    memset(response->body, 0, response->body_size);
    fread(response->body, response->body_size, 1, fileptr);
    fclose(fileptr);
    return;
}

void show_dir_content(struct http_response * response) {
    char * path = "static";
    char *html = "<html> <ul> %s </ul> </html>";
    char *ui = "<li><a href='/static/%s'>static/%s</a></li>";

    char liStr[500];
    char * liStrP = liStr;
    DIR *d = opendir(path); // open the path
    // if (d == NULL)
    //     return;                        // if was not able return
    struct dirent *dir;                // for the directory entries
    while ((dir = readdir(d)) != NULL) // if we were able to read somehting from the directory
    {
        if (dir->d_type != DT_DIR) { // if the type is not directory just print it with blue
            // printf("%s\n", dir->d_name);
            sprintf(liStrP, ui, dir->d_name, dir->d_name);
            liStrP = liStr + strlen(liStr);
        }
    }
    closedir(d); // finally close the directory
    response->body_size = (strlen(liStr) + strlen(html)) * sizeof(char);
    response->body = (char*)malloc(response->body_size);
    memset(response->body, 0, response->body_size);
    sprintf(response->body, html, liStr);
    return;
}

void doCgi(char * filePath, struct http_response * response) {
    char fileName[100];
    char cmd[100];
    sprintf(fileName, "cgi/%s", filePath + strlen(action_url + 1));
    sprintf(cmd, "%s 2>&1", fileName);

    FILE *fstream = NULL;
    if (access(fileName, F_OK) == -1 || NULL == (fstream = popen(cmd, "r"))) {
        // file doesn't exist or FILE cannot be exec
        setResponseMsg(response, errorMsg, fileName);
        return;
    }

    response->body = (char*)malloc((MaxResponseLen));
    memset(response->body, 0, MaxResponseLen);
    int len = 0;
    char *buff = response->body;
    do {
        buff += len;
        len = fread(buff, 1024, 1, fstream);
        // printf("%d\n", len);
    } while (len);
    pclose(fstream);
    response->body_size = strlen(response->body);
    struct Item *item2 = newItem(
        "Content-Type",
        "text/html; charset=utf-8");
    mapPush(response->headers, item2);

    return;
}
