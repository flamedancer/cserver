# 从零开始一个http服务器-动态返回（四)

代码地址 ： https://github.com/flamedancer/cserver
git checkout step4
运行：
make clean && make && ./myserver.out
测试:
浏览器打开 http://127.0.0.1:9734/

## response 返回文件 根据request 动态返回 response

* Makefile 
* 读取文件内容并作为body返回
* 列出目录下文件

###Makefile

看看我们现在的编译运行命令`gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out`

太长了！而且如果以后添加新的文件编译命令还要改。我们用Makefile来优化编译。

```makefile
objects = main.o request.o response.o \
    utils.o
VPATH = tools

myserver.out : $(objects)
	cc -o myserver.out $(objects)

main.o :
request.o :
response.o :
utils.o :

.PHONY : clean
clean :
	-rm myserver.out $(objects)
```

第1，2行定义了一个常量`objects`，因为要经常用到`main.o request.o response.o utils.o`，定义这个常量可以节省打字量。

`VPATH`是指除了当前路径外的额外路径，例如当前路径是没有utils.c的，需要告诉它还可以在tools目录找。

第5，6行是一组，第5行`:`冒号是前置声明，也就是声明如果要生成冒号前的文件，需要冒号后面那些文件存在。注意这只是声明。第6行开头需要有tag键（vim 可以在v模式下打出tab），然后是要执行的命令，注意这个命令不一定非要生成冒号前的目标文件。

第8~11行其实和第5行一样，只是利用了make的自动推导功能：根据 .o文件推导需要同名的.c文件，同时推导命令 cc -o main.o main.c

第14行也是一样的，只不过手动用.PHONY说明了下这里并不是要真正生成clean这个目标文件，只是为了执行后面的命令而已, rm 前的 - 号 是指遇到错误继续执行。

这样我们以后编译只要执行make就好了，会生成目标文件myserver.out，可以执行make clean 来清理中间文件。或者直接执行`make clean && make && ./myserver.out`来运行我们的程序。



###读取文件内容并作为body返回

现在我们发送给浏览器是固定的内容，当需要改变内容时，需要重新编译。这样很不灵活，也不实用。

我们修改逻辑，让服务器收到请求时，都去读取文件，再返回文件内容，这样的话当我们要改变发送内容时，只需要修改文件就好了。

``` c 
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
```

打开文件和关闭文件的函数 fopen fclose 很简单。

这里主要注意获取文件大小的方法：

​	fseek 移动文件指针，从文件末尾（SEEK_END代表文件末尾）移动0位置，跳到文件末尾。

​	ftell 获取文件首到当前文件指针的距离（偏移字节数）。这样就获取了文件大小。

​	rewind 再把文件指针移动回首。

Content-Length的真实意义是字节数，ftell返回的也是字节数，所以body_size不需要 * sizeof(char)

### 列出目录下文件

我们想在首页列出可以跳转的链接。为此可以扫描static目录下的所有文件，然后动态构造带<a>标签的html返回。

```c
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
```

这里主要是打开目录，遍历目录和关闭目录 opendir, readdir, closedir 这几个函数。



最后修改doResponse方法：

```c
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
```

执行 make clean && make && ./myserver.out 看看效果！



 
