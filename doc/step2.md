# 从零开始一个http服务器 （二)

## 解析http request

* 观察收到的http数据 
* 解析 request 的 method url version
* 解析 header 
* 解析 body 

### 观察收到的http数据
    上一节我们完成了一个简单的基于TCP/IP的socket server 程序。而HTTP正式基于TCP/IP的应用层协议，所以只要我们的程序能读懂HTTP数据，并做出符合HTTP协议的响应，那么就能完成HTTP的通信。
    上一节最后我们用telnet成功连接了我们的服务器，但只是向它传送了一些没有意义的字符。如果是浏览器，会传送什么呢？我们试着在浏览器地址栏输入我们的服务器地址： 127.0.0.1:9734 后访问，发现浏览器说“127.0.0.1 发送的响应无效。”, 那是说我们返回给浏览器的数据浏览器读不懂，因为现代的浏览器默认用http协议请求访问我们的服务器,而我们的返回的数据只是"helloworld"字符串，并不符合http协议的返回格式。虽然如此,但浏览器却是很有诚意的给我们的服务器发标准的http请求，不行我们看下我们的服务器收到的信息：
```
GET / HTTP/1.1
Host: 127.0.0.1:9734
Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8
```
    先观察一会儿,看起来第一行是http请求的类型，第二行开始是一些":"号分割的键值对。的确如此，第一行告诉我们是用的GET请求，请求的url是"/",用的是1.1的HTTP版本。第二行开始是HTTP的请求头部。
    除了GET请求外，另一种常用的请求是POST。用浏览器发POST请求稍麻烦，我们就借用curl工具来发送个HTTP POST请求给服务器看下数据又会是怎们样的：
`curl -d "message=nice to meet you" 127.0.0.1:9734/hello`, 服务器收到的信息：
```
POST /hell HTTP/1.1
Host: 127.0.0.1:9734
User-Agent: curl/7.54.0
Accept: */*
Content-Length: 24
Content-Type: application/x-www-form-urlencoded

message=nice to meet you
```
可以看到头部信息之后多了一空行和之后的POST的body数据信息。还要注意的是Content-Length头，代表POST的body数据的大小。

### 解析 request 的 method url version
    先来解析最简单的第一行: "POST /hell HTTP/1.1", 只需要用空格split出三个字符串就好了。  
``` c
// request.h
struct http_request {
    char * method;
    char * url;
    char * version;
};


void parse_request(
    struct http_request * request,
    char * http_data);
```
``` c
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
```
    编写测试用例:
``` c
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
```

    在test目录下执行:` gcc  ../request.h ../request.c requestTest.c && ./a.out`,可以看到我们解析的方法正确。

### 解析 header
    header的解析看起来比较复杂， 每一行倒是很容易看出是用":"分割的key-value对，所以我们可以用HashMap来表达，但这里的value可能有多个，数量不确定的，需要用链表来存储。如何判断header数据的结束呢，通过前面的观察，可以发现如果是POST会有一个空行和body隔开，但是GET的话只能检查客户端的数据是否发完，发完就代表header也结尾了。
    1. 创建链表结构体
    2. 创建哈希表结构体
    3. 实现字符串split方法
    4. 按行解析header，遇到空行或字符串结尾停止

1.   
首先声明链表结构体
    * 链表元素结构体，用来存放实际的值，再加一个指向下一个的指针
    * 代表链表的结构体，存放链表的关键属性如大小，头尾指针
``` c
/* tools/utils.h
*/
struct ListItem {
    struct ListItem* next;
    char* value;
};


struct List {
    struct ListItem* start;
    struct ListItem* end;
    int length;
};

```
再声明我们要用到的方法:新增元素，打印链表
``` c
void listAppend(struct List* list, struct ListItem* item);
void listPrint(struct List* List);
```

方法实现
``` c
#include <errno.h>        /* errno */
#include <stdio.h>       /* NULL */
#include "utils.h"

/* 在list尾端添加item
1. 若list为空，首尾都指向item
2. 否则，尾端的下一项指向item, 再置尾端为item
3. length + 1
*/
void listAppend(struct List* list, struct ListItem* item) {
    item->next = NULL;
    if(list->start == NULL) {
        list->start = list->end = item;
    } else {
        list->end->next = item;
        list->end = item;
    }
    list->length++;
}

void listPrint(struct List* list) {
    struct ListItem* point = list->start;
    printf("[");
    for(int i=0; i<list->length; i++) {
        if( i>0 ) {
            printf(", ");
        }
        printf("'%s'", point->value);
    }
    printf("]\n");
}
```
测试
``` c
/* test/utilsTest.c
    test cmd :
    gcc  ../tools/utils.h ../tools/utils.c utilsTest.c && ./a.out
*/
#include <assert.h>
#include <stdio.h>  /*  printf */
#include "../tools/utils.h"

void listAppendTest() {
    struct List list_instance;
    struct List* list = &list_instance;

    struct ListItem listItem_instance;
    struct ListItem* listItem = &listItem_instance;
    listItem->value = "hello world";

    struct ListItem listItem_instance2;
    struct ListItem* listItem2 = &listItem_instance2;
    listItem->value = "nice to meet you";


    assert(list->length == 0);
    listAppend(list, listItem);
    assert(list->length == 1);
    listAppend(list, listItem2);
    listPrint(list);
    printf("test listAppend OK\n");
}


int main() {
    listAppendTest();
}
```

