#ifndef __TOOLPOLL__
#include "../request.h"

#define __TOOLPOLL__

#define Readtrigger 1
#define Writetrigger 2
#define Emptytrigger 0
#define Unkonwtrigger 1

#define MaxEvents 20

struct PollEvent {
    int epfd;
    void * eventItems;
    int maxEventCnt;
};


int initPollEvent(struct PollEvent * event); /*  return result   -1  error   1 success  */
void releasePollEvent(struct PollEvent * even);
// void setNonBlock(int fd);

/**  add eventFLag   modify 为1 时  会 del不在eventFLag 的状态  暂时实现 read 和 write 两种状态 **/
void updateEvents(struct PollEvent* event, int fd, int eventFLags, int modify, void* udata);
int doPoll(struct PollEvent* event);

void * getIndexEventItem(void* eventItems, int n);
int getFid(void* eventItem);
int getEventType(void* eventItem);
void * getEventData(void* eventItem);

void setNonBlock(int fd);

/*
检查 eventItem 是否 有错误信息
*/
// int isChecked(void* eventItem);

#endif
