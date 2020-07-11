/*************************************************************************
	> File Name: client.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Wed 08 Jul 2020 04:32:12 PM CST
 ************************************************************************/

#include "head.h"

int server_port = 0;
char server_ip[20] = {0};
char *conf = "./football.conf";
int sockfd = -1;

WINDOW *message_win, *message_sub, *info_win;
WINDOW *info_sub, *input_win, *input_sub;
int msgnum = 0;

void logout(int signum) {
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void *)&msg, sizeof(msg), 0);
    close(sockfd);
    endwin();
    exit(0);
}

int main(int argc, char **argv) {
    setlocale(LC_ALL,"");
    int opt;
    struct LogRequest request;        
    struct LogResponse response;  

    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    request.team = -1;
      
    while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {
        switch (opt) {
            case 't':
                request.team = atoi(optarg);
                break;
            case 'h':
                strcpy(server_ip, optarg);
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'm':
                strcpy(request.msg, optarg);
                break;
            case 'n':
                strcpy(request.name, optarg);
                break;
            default:
                fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);
                exit(1);
        }
    }
    

    if (!server_port) server_port = atoi(get_conf_value(conf, "SERVERPORT"));
    if (request.team == -1) request.team = atoi(get_conf_value(conf, "TEAM")); //HERE
    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(conf, "SERVERIP"));
    if (!strlen(request.name)) strcpy(request.name, get_conf_value(conf, "NAME"));//HERE
    if (!strlen(request.msg)) strcpy(request.msg, get_conf_value(conf, "LOGMSG"));//HERE


    DBG("<"GREEN"Conf Show"NONE"> : server_ip = %s, port = %d, team = %s, name = %s\n%s\n",\
        server_ip, server_port, request.team ? "BLUE": "RED", request.name, request.msg);
    
    init_ui();

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    socklen_t len = sizeof(server);

    //init_ui();
    if ((sockfd = socket_udp()) < 0) {
        perror("socket_udp()");
        exit(1);
    }

    sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len);//HERE

    struct ChatMsg tmp;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if(select(sockfd + 1, &rfds, NULL, NULL, &tv) == 0) {
        perror("select()");
        exit(1);
    }

    int ret = recvfrom(sockfd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&server, &len);
    if (ret != sizeof(response) || response.type) {
        DBG(RED"Error: "NONE"The Game Server refused your login.\nThis May be helpful: %s\n", response.msg);
        exit(1);
    }
    DBG(GREEN"Server: "NONE"%s\n", response.msg);

    strcpy(tmp.msg, response.msg);
    show_message(message_sub, &tmp, 1);
    show_info(info_sub, &request);

    connect(sockfd, (struct sockaddr *)&server, len);
    
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, do_recv, NULL);

    signal(SIGINT, logout);
    struct ChatMsg msg;
    while (1) {
        echo();
        nocbreak();
        bzero(&msg, sizeof(msg));
        msg.type = CHAT_WALL;
        w_gotoxy_puts(input_win, 1, 1, "Input Message : ");
        strcpy(msg.name, request.name);
        //printf(YELLOW"Please Input :\n"NONE);
        //scanf("%[^\n]s", msg.msg);
        //getchar();
        wrefresh(input_win);
        mvwscanw(input_win, 2, 1, "%[^\n]s", msg.msg);
        if (strlen(msg.msg)) {
            if (msg.msg[0] == '@') msg.type = CHAT_MSG;
            if (msg.msg[0] == '#') msg.type = CHAT_FUNC;
            send(sockfd, (void *)&msg, sizeof(msg), 0);
        }
        wclear(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
        noecho();
        cbreak();
    }
    return 0;
}
