
#include <err.h>
#include <stdio.h> /* fprintf  NULL */
#include <sys/event.h> //  mac os
#include "poll.h"

const static int FD_NUM = 2;  // 两个文件描述符，分别为标准输入与输出

int initPollEvent(struct pollEvent* event)
{
    if ((event->epfd = kqueue()) == -1) {
        err(1, "Cannot create kqueue");
        return -1;
    }
    event->eventItems = (struct kevent*)malloc(MaxEvents * sizeof(struct kevent));
    event->maxEventCnt = MaxEvents;
    return 1;
}

void releasePollEvent(struct pollEvent* event) {
    free(event->eventItems);
}

void addEvents(struct pollEvent* event, int fd, int eventFLags, int modify, void* udata)
{
    struct kevent ev[2];
    int n = 0;
    if (eventFLags & kReadEvent) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    } else if (modify) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    }
    if (eventFLags & kWriteEvent) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    } else if (modify) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
    }

    int r = kevent(event, ev, n, NULL, 0, NULL);
    if (r == -1) {
        err(1, "set add events error ");
    }
}

int doPoll(struct pollEvent * event) {
    int n = kevent(event->epfd, NULL, 0, event->eventItems, event->maxEventCnt, NULL);
    return n;
}

int getFid(void * eventItem) {
    return ((struct kevent*)eventItem)->ident;
}

int getEventType(void * eventItem) {
    int type = ((struct kevent*)eventItem)->filter;
    int returnType = kUnkonwEvent;
    switch (type) {
        case EVFILT_READ:
            returnType = kReadEvent;
            break;
        case EVFILT_WRITE:
            returnType = kWriteEvent;
            break;
        default:
            returnType = kUnkonwEvent;
            break;
    }
    return returnType;
}