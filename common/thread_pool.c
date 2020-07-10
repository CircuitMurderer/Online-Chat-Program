/*************************************************************************
	> File Name: thread_pool.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
extern int repollfd, bepollfd;
extern struct User *rteam, *bteam;
extern pthread_mutex_t rmutex, bmutex;

void disp_list(struct User *rteam, struct User *bteam, struct User *user) {
    
    struct ChatMsg msg;
    bzero(&msg, sizeof(msg));
    msg.type = CHAT_SYS;
    
    for (int i = 0; i < MAX; i++) {
        if (rteam[i].online) {
            bzero(msg.msg, sizeof(msg.msg));
            sprintf(msg.msg, "Red team < %s > is online!", rteam[i].name);
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        } else if (bteam[i].online) {
            bzero(msg.msg, sizeof(msg.msg));
            sprintf(msg.msg, "Blue team < %s > is online!", bteam[i].name);
            send(user->fd, (void *)&msg, sizeof(msg), 0);
        }
    }

}

void do_work(struct User *user){
    //
    //收到一条信息，并打印。
    struct ChatMsg msg;
    struct ChatMsg r_msg;

    bzero(&msg, sizeof(msg));
    bzero(&r_msg, sizeof(r_msg));

    recv(user->fd, (void *)&msg, sizeof(msg), 0);
    if (msg.type & CHAT_WALL) {
        printf(L_BLUE" < %s > "NONE"%s\n", user->name, msg.msg);
        send_all(&msg);
    } else if (msg.type & CHAT_MSG) {
        char to[20] = {0};
        int i;
        for (i = 1; i <= 21; i++) {
            if(msg.msg[i] == ' ') break;
        }
        if (msg.msg[i] != ' ' || msg.msg[0] != '@') {
            memset(&r_msg, 0, sizeof(r_msg));
            r_msg.type = CHAT_SYS;
            sprintf(r_msg.msg, "MESSAGE:Bad format!");
            send(user->fd, (void *)&r_msg, sizeof(r_msg), 0);
        } else {
            msg.type = CHAT_MSG;
            strncpy(to, msg.msg + 1, i - 1);
            send_to(to, &msg, user->fd);
        }
    } else if (msg.type & CHAT_FUNC) {
        if (msg.msg[0] != '#' || msg.msg[2] != ' ') {
            memset(&r_msg, 0, sizeof(r_msg));
            r_msg.type = CHAT_SYS;
            sprintf(r_msg.msg, "FUNCTION:Bad format!");
            send(user->fd, (void *)&r_msg, sizeof(r_msg), 0);
        } else {
            switch(msg.msg[1]){
                case '1': {
                    disp_list(rteam, bteam, user);
                    break;
                } default: {
                    memset(&r_msg, 0, sizeof(r_msg));
                    r_msg.type = CHAT_SYS;
                    sprintf(r_msg.msg, "FUNCTION:Unknown command!");
                    send(user->fd, (void *)&r_msg, sizeof(r_msg), 0);
                    break;
                }
            }
        } 
    } else if (msg.type & CHAT_FIN) {
        bzero(msg.msg, sizeof(msg.msg));
        msg.type = CHAT_SYS;
        sprintf(msg.msg, "User < %s > has logged out!", msg.name);
        send_all(&msg);

        if (user->team) pthread_mutex_lock(&bmutex);
        else pthread_mutex_lock(&rmutex);

        user->online = 0;
        int epollfd = user->team ? bepollfd :repollfd;
        del_event(epollfd, user->fd);

        if (user->team) pthread_mutex_unlock(&bmutex);
        else pthread_mutex_unlock(&rmutex);

        printf(GREEN"Server Info : "NONE"%s logout!\n", user->name);
        close(user->fd);
    }
}

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd) {
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void *));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user) {
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}


struct User *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User *user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void *thread_run(void *arg) {
    pthread_detach(pthread_self());
    struct task_queue *taskQueue = (struct task_queue *)arg;
    while (1) {
        struct User *user = task_queue_pop(taskQueue);
        do_work(user);
    }
}

