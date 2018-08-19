# 从零开始一个http服务器（三)
代码地址 ： https://github.com/flamedancer/cserver
git checkout step3
运行：
gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out
测试:
浏览器打开 http://127.0.0.1:9734/
## response 构造

* 观察response结构 
* 定义并返回response
* 测试 

###观察response结构 

上一节,我们成功解析了http的request，但是我们在浏览器访问我们的地址http://127.0.0.1:9734/ 还是无法正常显示。这是因为我们没有给浏览器返回它能读懂的信息。这一节我们的目标是让浏览器正确的显示信息。什么样的才是浏览器能读懂的信息呢？不妨我们用telnet来模拟向百度主页发一个http request,来看看百度主页返回的是什么信息。
伪造一个http request的字符串，注意 headers 中的 Host 代表我们要访问的主机地址。
```
GET / HTTP/1.1
Host: www.baidu.com
User-Agent: curl/7.54.0
Accept: */*
Content-Type: application/x-www-form-urlencode
```
再用telnet连接www.baidu.com 并指定80端口（80为http默认端口，telnet默认端口为23）, `telnet www.baidu.com 80`
复制黏贴上面我们构造的字符串回车后，你应该能看到如下类似的返回结果：
```
Trying 119.75.216.20...
Connected to www.a.shifen.com.
Escape character is '^]'.
GET / HTTP/1.1
Host: www.baidu.com
User-Agent: curl/7.54.0
Accept: */*
Content-Type: application/x-www-form-urlencode

HTTP/1.1 200 OK
Accept-Ranges: bytes
Cache-Control: private, no-cache, no-store, proxy-revalidate, no-transform
Connection: Keep-Alive
Content-Length: 2381
Content-Type: text/html
Date: Sat, 18 Aug 2018 02:12:08 GMT
Etag: "588604c8-94d"
Last-Modified: Mon, 23 Jan 2017 13:27:36 GMT
Pragma: no-cache
Server: bfe/1.0.8.18
Set-Cookie: BDORZ=27315; max-age=86400; domain=.baidu.com; path=/

<!DOCTYPE html>
<!--STATUS OK--><html> ...</html>
Connection closed by foreign host.
```
从`HTTP/1.1 200 OK`开始就是百度返回给我们的结果。让人惊喜的是这种结构和request很类似，除了第一行外。仔细看看: 
* 第一行为    http版本号 response返回码 response返回结果描述
* 第二行开始为headers
* 空行后，接body



###定义并返回response
根据我们观察到的response结构定义相对应的结构体.

``` c 
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
    FILE * stream
);
```

构造我们的response数据, 我们每次都返回相同的数据.

``` c
/* response.c
*/
#include <stdio.h>       /* fprintf  NULL */
#include <string.h>       /* strlen */
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

    
    char * content = "<html>hello everyone</html>";
    char content_len[25];
    sprintf(content_len, "%lu", strlen(content));
    struct Item * item = newItem(
        "Content-Length",
        content_len
    );
    struct Map map_instance;
    initMap(&map_instance);
    response->headers = &map_instance;
    
    mapPush(response->headers, item);
    
    response->body = content;
    
    outputToFile(response, stream);
    
    // clean
    releaseMap(request->headers);
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
    if(response->body != NULL) {
        fprintf(stream, "\r\n%s", response->body);
    }
}
```

写一个测试用例，将本应向客服端发送的数据输出到stdout
``` c
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

```
cd 到 test 目录
运行： `gcc  ../request.h ../request.c ../response.h ../response.c ../tools/utils.h ../tools/utils.c  responseTest.c && ./a.out`
可以看到正确的输出：
```
---------------------------
method is: POST
url is: /
http version is: HTTP/1.1
the headers are :
{'Content-Length': ' 3'}
body is 111
---------------------------
HTTP/1.1 200 OK
Content-Length: 27

<html>hello everyone</html>
```

现在修改main函数，加上我们的reponse处理逻辑
``` c
/**
run cmd:
gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out

*/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>

#include "request.h"
#include "response.h"
#define MAXREQUESTLEN 50000


void initString(char * c, int length) {
    int i = 0;
    while(i < length) {
        *(c + i) = '\0';  
        i++;
    }
}

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

        char ch[MAXREQUESTLEN];
        initString(ch, MAXREQUESTLEN); 
        // char send_str[] = "hello world !\n";
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
        (struct sockaddr *)&client_address, &client_len);

        read(client_sockfd, &ch, MAXREQUESTLEN);
        printf("%s\n", ch);
        struct http_request request;
        struct Map headers;
        request.headers = &headers;
        parse_request(&request, ch); 
        FILE* fp = fdopen(client_sockfd, "w+");
        doResponse(&request, fp); 
        fflush(fp);
        fclose(fp);
        // write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0])); 
    }
}

```

###测试
启动我们的server `gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out`
再在浏览器访问我们的服务器地址 http://127.0.0.1:9734/
现在浏览器能识别我们的返回结果了！

 
