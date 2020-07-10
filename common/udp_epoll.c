/*************************************************************************
	> File Name: udp_epoll.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 04:40:38 PM CST
 ************************************************************************/

#include "head.h"
extern int port;
extern struct User *rteam;
extern struct User *bteam;
extern int repollfd, bepollfd;

int udp_connect(struct sockaddr_in *client) {
    int sockfd;
    if ((sockfd = socket_create_udp(port)) < 0) {
        perror("socket_create_udp");
        return -1;
    }
    if (connect(sockfd, (struct sockaddr *)client, sizeof(struct sockaddr)) < 0) {
        return -1;
    }
    return sockfd;
}


int udp_accept(int fd, struct User *user) {
    int new_fd, ret;
    struct sockaddr_in client;
    struct LogRequest request;
    struct LogResponse response;
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    socklen_t len = sizeof(client);
    
    ret = recvfrom(fd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&client, &len);
    
    if (ret != sizeof(request)) {
        response.type = 1;
        strcpy(response.msg, "Login failed with Data errors!");
        sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
        return -1;
    }
    
   /* if (check_online(&request)) {
        response.type = 1;
        strcpy(response.msg, "You are Already Login!");
        sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
        return -1;
    }*/

    response.type = 0;
    strcpy(response.msg, "Login Success. Enjoy yourself!");
    sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);
    
    if (request.team) {
        DBG(GREEN"Info"NONE" : "BLUE"%s login on %s:%d  <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);
    } else {
        DBG(GREEN"Info"NONE" : "RED"%s login on %s:%d   <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);
    }

    strcpy(user->name, request.name);
    user->team = request.team;
    new_fd = udp_connect(&client);
    user->fd = new_fd;
    return new_fd;
}

void add_event_ptr(int epollfd, int fd, int events, struct User *user） {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void *)user;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void del_event(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}
int find_sub(struct User *team) {
    for (int i = 0; i < MAX; i++) {
        if (!team[i].online) retrun i;
    }
    return -1;
}

void add_to_sub_reactor(struct User *user) {
    //根据user里的team变量判断是红队还是蓝队，进而知道用户存储的数组是rteam, 还是bteam
    int tk = user->team;
    //find_sub(team);
    int sub;
    //将user指向的用户信息存放在team[sub]中
    if (tk == 1) {
        sub = find_sub(bteam);
        bteam[sub] = *user;
        bteam[sub].online = 1;
        btesm[sub].flag = 10;
        add_event_ptr(bepollfd, user->fd, EPOLLIN | EPOLLET, user);
    } else if (tk == 0) {
        sub = find_sub(rteam);
        rteam[sub] = *user;
        rteam[sub].online = 1;
        rtesm[sub].flag = 10;
        add_event_ptr(repollfd, user->fd, EPOLLIN | EPOLLET, user);
    }
    //根据user->team不同，将用户加到不同的从反应堆中，使用add_event_ptr函数。注册EPOLLIN 和 EPOLLET事件
}

