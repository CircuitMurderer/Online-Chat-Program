/*************************************************************************
	> File Name: client_recv.c
	> Author: 
	> Mail: 
	> Created Time: Fri 10 Jul 2020 06:02:36 PM CST
 ************************************************************************/

#include "head.h"

extern int sockfd;
extern WINDOW *message_sub;

void *do_recv(void *arg) {
	while (1) {
		struct ChatMsg msg;
		bzero(&msg, sizeof(msg));
		recv(sockfd, (void *)&msg, sizeof(msg), 0);
		if (msg.type & CHAT_WALL) {
			//printf(""BLUE"%s"NONE" : %s\n", msg.name, msg.msg);
			show_message(message_sub, &msg, 0);
		} else if (msg.type & CHAT_MSG) {
			//printf(""RED"%s"NONE" : %s\n", msg.name, msg.msg);
			show_message(message_sub, &msg, -1);
		} else if (msg.type & CHAT_SYS) {
			//printf(YELLOW"Server Info"NONE" : %s\n", msg.msg);
			show_message(message_sub, &msg, 1);
		} else if (msg.type & CHAT_FIN) {
			//printf(L_RED"Server Info"NONE" : Server down!\n");
			endwin();
			exit(1);
		}
	}
}
