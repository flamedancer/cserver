# cserver
从零开始一个服务器（C语言）
# 运行
gcc request.h request.c response.h response.c main.c tools/utils.c tools/utils.h && ./a.out
# 浏览器访问 http://127.0.0.1:9734/

plan
* hello world 开始 【完成】 
* request 解析 【完成】
* response 构造 【完成】
* response 返回文件
* 根据request 动态返回 response
* 用外部程序来简化 动态 response 
* 压力测试 和 多路复用
* 线程池？
