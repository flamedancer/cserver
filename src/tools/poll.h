#ifndef __TOOLPOLL__
#define __TOOLPOLLS__


const int kReadEvent = 1;
const int kWriteEvent = 2;
const int kEmptyEvent = 0;
const int kUnkonwEvent = -1;

const int MaxEvents = 20;

struct pollEvent {
    int epfd;
    void * eventItems;
    int maxEventCnt;
};

int initPollEvent(struct pollEvent * event); /*  return result   -1  error   1 success  */
void releasePollEvent(struct pollEvent* even);
int doPoll(struct pollEvent* event);
// void setNonBlock(int fd);

/**  modify 为1  表示 del不在eventFLag 的 状态否则只是 add ventFLag **/
void addEvents(struct pollEvent* event, int fd, int eventFLags, int modify, void* udata);

int getFid(void* eventItem);
int getEventType(void* eventItem);

#endif
