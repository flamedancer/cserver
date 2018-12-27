
#include "../poll.h"
#include "../../config.h"
#include <err.h>
#include <fcntl.h>
#include <stdio.h> /* fprintf  NULL */
#include <stdlib.h> /* mallco */

#include <sys/event.h> //  mac os

int initPollEvent(struct PollEvent* event)
{
    if ((event->epfd = kqueue()) == -1) {
        err(1, "Cannot create kqueue");
        return -1;
    }
    event->eventItems = (struct kevent*)malloc(MaxEvents * sizeof(struct kevent));
    event->maxEventCnt = MaxEvents;
    return 1;
}

void releasePollEvent(struct PollEvent* event)
{
    free(event->eventItems);
}

/*
It's not alwas nessesary to explicitly delete kqueue filters, because calling close() on a file descriptor will remove any kevents that reference the descriptor
*/
void updateEvents(struct PollEvent* event, int fd, int eventFLags, int policy, int modify, void* udata)
{
    switch (policy) {
    case TriggerPolicy_ONESHOT:
        policy = EV_ONESHOT;
        break;
    case TriggerPolicy_CLEAR:
        policy = EV_CLEAR;
        break;
    default:
        // always
        policy = 0;
    }

    struct kevent ev[2];
    int n = 0;
    if (eventFLags & Readtrigger) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE | policy, 0, 0, udata);
    } // else if (modify) {
    //      EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    // }
    if (eventFLags & Writetrigger) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE | policy, 0, 0, udata);
    } // else if (modify) {
    //      EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
    // }

    int r = kevent(event->epfd, ev, n, NULL, 0, NULL);
    debug_print("updateEvent: epfd is %d, fd is %d, flag is %d, result is %d \n", event->epfd, fd, eventFLags, r);
    if (r) {
        err(1, "kevent failed ");
    }
}

int doPoll(struct PollEvent* event)
{
    int n = kevent(event->epfd, NULL, 0, event->eventItems, event->maxEventCnt, NULL);
    return n;
}

void* getIndexEventItem(void* eventItems, int n)
{
    return (void*)(((struct kevent*)eventItems) + n);
}

int getFid(void* eventItem)
{
    return ((struct kevent*)eventItem)->ident;
}

int getEventType(void* eventItem)
{
    int type = ((struct kevent*)eventItem)->filter;
    int returnType = Unkonwtrigger;
    switch (type) {
    case EVFILT_READ:
        returnType = Readtrigger;
        break;
    case EVFILT_WRITE:
        returnType = Writetrigger;
        break;
    default:
        returnType = Unkonwtrigger;
        break;
    }
    return returnType;
}

void* getEventData(void* eventItem)
{
    return ((struct kevent*)eventItem)->udata;
}

void setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// int isChecked(void* eventItem) {
//     return 1;
// }
