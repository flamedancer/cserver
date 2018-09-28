
/* response.c
*/
#include <stdio.h>       /* fprintf  NULL */
#include <string.h>       /* strlen */
#include <stdlib.h>     /* malloc, free, rand */
#include <dirent.h>     /* DIR opendir DT_DIR */
#include "response.h"
#include "request.h"
#include "tools/utils.h"

void initHttpResponse(struct http_response * response) {
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
    response->version = "HTTP/1.1";
    response->code = "200";
    response->desc = "OK";

    char content_len[25];

    char * home_url = "/";
    char * static_url = "/static/";
    char * action_url = "/action/";

    if (strncmp(static_url, request->url, strlen(static_url)) == 0) {
        responeFileContent(request->url + 1, response);
    }
    else if (strncmp(home_url, request->url, strlen(home_url)) == 0) {
        show_dir_content(response);
    } else {
        char *content = "<html><meta charset='utf-8'><a src='/'> >_< 看来你迷路了 </a></html>";
        response->body_size = (int)strlen(content);
        response->body = (char *)malloc((response->body_size));
        strcpy(response->body, content);
    }
    sprintf(content_len, "%d", response->body_size);
    printf("body size is %d\n", response->body_size);
    struct Item * item = newItem(
        "Content-Length",
        content_len
    );
    struct Map map_instance;
    initMap(&map_instance);
    response->headers = &map_instance;
   
    mapPush(response->headers, item);
    
    outputToFile(response, stream);

    // clean
    free(response->body);   // If ptr is NULL, no operation is performed.
    response->body = NULL;
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
                item->value
            );
            item = item->next;
        }
    }
    // output body
    if(response->body_size > 0 && response->body != NULL) {
        fprintf(stream, "\r\n");
        fwrite(response->body, response->body_size, 1, stream);
    }
}

void responeFileContent(char * filePath, struct http_response * response) {
    char * error_file = "static/404.html";
    FILE * fileptr;
    fileptr = fopen(filePath, "rb");
    if (NULL == fileptr)
    {
        fileptr = fopen(error_file, "rb");
    }
    fseek(fileptr, 0, SEEK_END);
    response->body_size = ftell(fileptr);
    rewind(fileptr);
    response->body = (char *)malloc((response->body_size));
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
            printf("%s\n", dir->d_name);
            sprintf(liStrP, ui, dir->d_name, dir->d_name);
            liStrP = liStr + strlen(liStr);
        }
    }
    closedir(d); // finally close the directory

    response->body = (char *)malloc((strlen(liStr) + strlen(html)) * sizeof(char));
    sprintf(response->body, html, liStr);
    response->body_size = strlen(response->body);
    return;
}
