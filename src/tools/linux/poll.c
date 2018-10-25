
#include "../poll.h"
#include "../../config.h"
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>

int initPollEvent(struct PollEvent* event)
{
    if ((event->epfd = epoll_create1(0)) == -1) {
        err(1, "Cannot create epoll");
        return -1;
    }
    event->eventItems = (struct epoll_event*)calloc(MaxEvents, sizeof(struct epoll_event));
    event->maxEventCnt = MaxEvents;
    return 1;
}

void releasePollEvent(struct PollEvent* event)
{
    free(event->eventItems);
}

/*
man 7 epoll

       Q6  Will closing a file descriptor cause it to be removed from all
           epoll sets automatically?

       A6  Yes, but be aware of the following point.  A file descriptor is a
           reference to an open file description (see open(2)).  Whenever a
           file descriptor is duplicated via dup(2), dup2(2), fcntl(2)
           F_DUPFD, or fork(2), a new file descriptor referring to the same
           open file description is created.  An open file description con‐
           tinues to exist until all file descriptors referring to it have
           been closed.  A file descriptor is removed from an epoll set only
           after all the file descriptors referring to the underlying open
           file description have been closed (or before if the file descrip‐
           tor is explicitly removed using epoll_ctl(2) EPOLL_CTL_DEL).
           This means that even after a file descriptor that is part of an
           epoll set has been closed, events may be reported for that file
           descriptor if other file descriptors referring to the same under‐
           lying file description remain open.
*/
void updateEvents(struct PollEvent* event, int fd, int eventFLags, int modify, void* udata)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.data.ptr = udata;
    int mod = EPOLL_CTL_ADD;
    if (modify) {
        mod = EPOLL_CTL_MOD;
    } else {
        mod = EPOLL_CTL_ADD;
    }
    if (eventFLags == Readtrigger) {
        ev.events = EPOLLIN | EPOLLET;
    } else if (eventFLags == Writetrigger) {
        ev.events = EPOLLOUT | EPOLLET;
    }

    int r = epoll_ctl(event->epfd, mod, fd, &ev);
    debug_print("updateEvent: epfd is %d, fd is %d, flag is %d, result is %d \n", event->epfd, fd, eventFLags, r);
    if (r) {
        err(1, "epoll failed ");
    }
}

int doPoll(struct PollEvent* event) {
    int n = epoll_wait(event->epfd, event->eventItems, event->maxEventCnt, -1);
    if (n == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }
    return n;
}

void* getIndexEventItem(void* eventItems, int n)
{
    return (void*)(((struct epoll_event*)eventItems) + n);
}

int getFid(void* eventItem)
{
    return (((struct epoll_event*)eventItem)->data).fd;
}

int getEventType(void* eventItem)
{
    int flag = ((struct epoll_event*)eventItem)->events;
    int returnType = Unkonwtrigger;

    if (flag & EPOLLIN) {
        returnType = Readtrigger;
        return returnType;
    } else if (flag & EPOLLOUT) {
        returnType = Writetrigger;
        return returnType;
    }
    return returnType;
}

void* getEventData(void* eventItem)
{
    return ((struct epoll_event*)eventItem)->data.ptr;
}

/* Linux
*/
void setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

