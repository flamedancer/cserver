# 从零开始一个http服务器(六)-多路复用和压力测试

代码地址 ： https://github.com/flamedancer/cserver
git checkout step6
运行：
make clean && make && ./myserver.out
测试
浏览器打开 http://127.0.0.1:9734/action/show_date

## 多路复用和压力测试
* 多路复用
* 压力测试

### 多路复用

####多路复用意义

到目前为止，服务器的基本业务功能我们是实现了。但好的服务器不止要能完成业务逻辑，没有好的执行效率和高的并发数，根本无法在生成环境中使用。现在我们开始考虑优化我们的服务器使之接近能应付生成环境的水平。

提高服务器性能有很多方法，我们先来考虑第一个改进，采用多路复用。

我们之前的程序，当server_sockfd accpet 一个 client_sockfd 后，程序read等待client_sockfd发来的数据，

然后再解析request。然而如果client_sockfd迟迟不发送数据，我们的程序就卡在read上，后面客户端发的请求全部无法响应了。"多路复用"可以简单理解为“谁有事执行谁”， 运用多路复用可以解决上面这个问题。当accpet 一个 client_sockfd后，将client_sockfd放在一个事件池中，直到事件池察觉这个client_sockfd可以read了，才对这个client_sockfd进行read操作。

步骤可以总结为

   	1. 创建事件池
   	2. 创建事件，将事件加入到事件池
   	3. 事件池监测所有事件，若有事件被触发，执行被触发事件的后续逻辑



####多路复用封装

由于mac和linux所用的事件复用库不一样，mac下为kqueue，linux下为epoll，为了兼容性，我们封装了一个事件库poll.h。
主要结构体和函数为：

#####1 . 事件池结构体

```c
struct PollEvent {
    int epfd;
    void * eventItems;
    int maxEventCnt;
};
```
epfd 为事件池句柄
eventItems 当前被触发的事件数组
maxEventCnt 为最大可容纳被触发事件数

#####2 . 添加事件
```c
void updateEvents(struct PollEvent* event, int fd, int eventFLags, int modify, void* udata);
```
event 事件池
fd 需要监听的句柄
eventFLags 需要监听fd的什么事件  我们初略的只定义读和写事件
```c
#define Readtrigger 1
#define Writetrigger 2
#define Emptytrigger 0
```
modify 如果为 0 时  为fd添加eventFLags事件, 为1时  修改fd的事件为eventFLags
#####3. 监听事件池

当有事件被触发时返回被触发的事件数，被触发的事件放在event->eventItems里
```c
int doPoll(struct PollEvent* event);
```

#### 修改main函数

1. 创建事件池
2. 将sever_socket  read 事件加入事件池
3. 监听事件池
4. 当事件池发现有触发的事件时，判断触发的事件类型：
   1. 若为 sever_socket  read  事件，创建client_socket, 将client_socket  read 事件加入事件池
   2. 若为 client_socket  read  事件,   解析reques，生成 response，并将client_socket  write 事件加入事件池
   3. 若为 client_socket  write  事件,  将response写入client_socket，关闭client_socket（事件池会清理关闭了的fd）
5. 继续第4步

###压力测试

 ab -c 5 -n 2000 http://127.0.0.1:9734/action/show_date

```
Concurrency Level:      5
Time taken for tests:   14.089 seconds
Complete requests:      2000
Failed requests:        0
Total transferred:      218000 bytes
HTML transferred:       58000 bytes
Requests per second:    141.95 [#/sec] (mean)
Time per request:       35.223 [ms] (mean)
Time per request:       7.045 [ms] (mean, across all concurrent requests)
Transfer rate:          15.11 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       0
Processing:    10   35  14.6     38      83
Waiting:       10   35  14.6     38      83
Total:         10   35  14.6     39      83
```

我们用ab测试5并发总共2000个/show_date请求，可以看到 平均每个请求花费35毫秒。下一节我们将采用线程池技术来继续改机我们的执行效率。