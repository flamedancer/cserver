# 从零开始一个http服务器-模拟cgi（五)

代码地址 ： https://github.com/flamedancer/cserver
git checkout step5
运行：
make clean && make && ./myserver.out
测试
浏览器打开 http://127.0.0.1:9734/action/show_date

##模拟cgi：用外部程序来优化 动态 response

* cgi解释 
* 调用外部程序

###cgi解释

上一节中，我们确实是实现了动态的response：我们不需要修改我们的代码，也不需要中断server，只需要修改我们的html页面文件就可以实时的更改返回内容。

但是当需要返回更加灵活的内容，比如当前时间的时候，我们不可能每隔一秒钟就去改下页面文件。这时候就可以借助外部的程序，比如shell，来为我们生产返回内容。这个外部程序，就类似于常说的cgi程序。所谓cgi，维基百科是这么解释：**通用网关接口**（**C**ommon **G**ateway **I**nterface/**CGI**）是一种重要的互联网技术，可以让一个客户端，从网页浏览器向执行在网络服务器上的程序请求数据。CGI描述了服务器和请求处理程序之间传输数据的一种标准。

这是说CGI是一种标准，只要服务器 和 外部的程序 都实现了这个标准，就可以相互通信。

我们这比较粗鲁一点，我们不管正规的CGI标准是什么，我们定义一个我们自己简单粗暴的标准，这个标准只有两条：

 1.  外部的程序 可以产生 标准输出

 2.  服务器可以获得外部的程序产生的标准输出

这样的话我们就可以把外部的程序的标准输出直接作为response的body。


###调用外部程序
执行外部程序的核心函数为popen ，它会开启一个新进程执行传入的外部命令，返回一个管道文件流，读取这个管道文件流就可以读取到外部命令的输出。管道文件流需要用pclose关闭，而不是fclose。

```c
void doCgi(char * filePath, struct http_response * response) {
    char fileName[100];
    char cmd[100];
    sprintf(fileName, "cgi/%s", filePath + strlen(action_url + 1));
    sprintf(cmd, "%s 2>&1", fileName);

    FILE *fstream = NULL;
    if (access(fileName, F_OK) == -1 || NULL == (fstream = popen(cmd, "r"))) {
        // file doesn't exist or FILE cannot be exec
        setResponseMsg(response, errorMsg);
        return;
    }

    response->body = (char *)malloc((5000));
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
```



建一个目录名为cgi,把我们写的外部程序放到这个目录，例如我们写个 cgi/show_date  程序：

```shell
#! /bin/bash
echo $(date)

```

make clean && make && ./myserver.out    打开  http://127.0.0.1:9734/action/show_date  可以看到实时的时间

 
