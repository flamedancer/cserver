# 从零开始一个http服务器 （一)

##一个简单的socket server

* 从helloworld开始
* 回顾c语言的socket 通信
* 一个简单的socket server
* 用telent测试 

###从helloworld 开始

先来回顾下c语言的，c语言的helloword程序如下
``` c
// main.c
#include<stdio.h>

int main() {
    printf("hello world");
}
```
编译 gcc main.c
运行 ./a.out
输出 hello world 



###回顾c语言的socket 通信

####socket (server端) 通信的一般 步骤

    * 创建 socket , 返回socket 文件描述符,需指明域(本地文件socket还是网络socket),类型(TCP 还是 UDP)
    * 绑定 bind, 绑定socket地址(本地socket文件地址 或 网络地址 IP + port) 
    * 监听 listen, 为socket创建监听队列, 连接到socket的链接将会进入这个队列, 需要指明队列最大长度
    * 接收链接 accept, 接收客户端链接，返回接收到的 客户socket文件描述符
    * 读写  read/write,  对 客户socket文件描述符 进行 读写操作来进行通信
    * close, 通信结束， 关闭 客户socket文件描述符, 整个server结束，也要关闭 server socket文件描述符
#### c 语言 socket通信有关的函数及结构原型 
1. creating a socket
```
    #include <sys/types.h> #include <sys/socket.h>
    int socket(int domain, int type, int protocol);

     *** domains
             AF_UNIX: 本地文件socket (file system sockets)
             
             AF_INET: 网络socket (UNIX network sockets)
             ...
     *** type
             SOCK_STREAM: TCP 协议 
             SOCK_DGRAM: UDP 协议
     *** protocol 
         一般选默认值 0
```
2. struct: socket Address socket 地址结构体
    本地文件socket地址: 
```
    AF_UNIX socket_un    defind in sys/un.h
       struct sockaddr_un {
           sa_family_t sun_family; // AF_UNIX
           char sun_path[]; // pathname
       }; 

    网络socket 地址:
    AF_INET sockaddr_in   defind in netinet/in.h
        struct sockaddr_in {
            short int sin_family;  // AF_INET
            unsigned short in sin_port;   // Port number
            struct in_addr sin_addr;  // Inernet address
        };
        其中代表ip地址的结构体in_addr:
        struct in_addr {
            unsigned long int s_addr;
        }
```
3. bind  
    成功返回0,失败返回-1,失败信息见 errno
```
    #include <sys/socket.h>
    int bind(int socket, const struct sockaddr *address, size_t address_len);
```

4. Creating a socket queue
```
    #include <sys/socket.h>
    int listen(int socket, int backlog);  // backlog : the maximum number of pending connections
```

5. Accept connections
    这里的address和address_len 都是指client端的地址,如果成功连接client,则address被填充
    返回连接后client 的 socket 文件描述符
```
    #include <sys/socket.h>
    int accept(int socket, struct sockaddr *address, size_t *address_len);
```
6. Host and Network Byte Ordering 
    有可能本地字节编码顺序和网络字节编码顺序不同，本地字节编码要转成网络字节编码
```
    #include <netinet/in.h>
    unsigned long int htonl(unsigned long int hostlong);
    unsigned short int htons(unsigned short int hostshort);
```

### 一个简单的socket server

``` c
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0); //  创建socket，选择地址类型为网络地址，选择 TCP 通信

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // 设置网络地址的ip, inet_addr 会自动 转为 网络字节顺序
    server_address.sin_port = htons(9734);    //  设置端口号，注意这里的 htons 方法 
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);
    while(1) {
        char ch[5000];
        char send_str[] = "hello world !\n";  // 准备给连接过来的客户端发送的字符串
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd,
        (struct sockaddr *)&client_address, &client_len);
        read(client_sockfd, &ch, 5000);    // 接收 客户端传来的字符
        printf("%s", ch);     //  打印我们接收到的字符
        write(client_sockfd, &send_str, sizeof(send_str)/sizeof(send_str[0]));   // 向客户端发送数据，这里的 read write 和 和文件读写时没什么区别 
        close(client_sockfd);
    }
}
```

和之前helloword一样编译运行我们的第一个版本！

### 用 telnet 测试 
看看效果吧！新启一个终端，然后用telnet 尝试连接我们的服务器。
执行命令 telnet 127.0.0.1 9734   
随便输入几个字符按回车
屏幕输出大概为这样：
```
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
dsfsd
hello world !
Connection closed by foreign host.
```
再返回查看我们的服务器屏幕打印，能看到我们刚才随意输入的字符，说明我们的服务器能成功接收并返回数据了。






