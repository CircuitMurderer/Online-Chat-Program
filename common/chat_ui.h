/*************************************************************************
	> File Name: chat_ui.h
	> Author: 
	> Mail: 
	> Created Time: Sat 11 Jul 2020 12:10:05 PM CST
 ************************************************************************/

#ifndef _CHAT_UI_H
#define _CHAT_UI_H

#define MSG_WIDTH 85
#define INFO_WIDTH 25
#define MSG_HEIGHT 25
#define INPUT_HEIGHT 5
void destroy_win(WINDOW *win);
void gotoxy(int x, int y);
void gotoxy_putc(int x, int y, int c);
void gotoxy_puts(int x, int y, char* s);
void w_gotoxy_putc(WINDOW *win, int x, int y, int c);
void w_gotoxy_puts(WINDOW *win, int x, int y, char *s);
void init_ui();
void show_message(WINDOW *win, struct ChatMsg *msg, int type);
void show_info(WINDOW *win, struct LogRequest *request);

#endif
