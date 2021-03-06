/*************************************************************************
	> File Name: chat_ui.c
	> Author: 
	> Mail: 
	> Created Time: Sat 11 Jul 2020 12:09:57 PM CST
 ************************************************************************/

#include "head.h"
extern WINDOW *message_win, *message_sub,  *info_win, *input_win, *info_sub, *input_sub;
extern int msgnum;

WINDOW *create_newwin(int width, int heigth, int startx, int starty) {
    WINDOW *win;
    win = newwin(heigth, width, starty, startx);
    box(win, 0, 0);
    wrefresh(win);
    return win;
}

void destroy_win(WINDOW *win) {
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(win);
    delwin(win);
}

void gotoxy(int x, int y) {
    move(y, x);
}

void gotoxy_putc(int x, int y, int c) {
    move(y, x);
    addch(c);
    move(LINES - 1, 1);
    refresh();
}

void gotoxy_puts(int x, int y, char* s) {
    move(y, x);
    addstr(s);
    move(LINES - 1, 1);
    refresh();
}

void w_gotoxy_putc(WINDOW *win, int x, int y, int c) {
    mvwaddch(win, y, x, c);
    move(LINES - 1, 1);
    wrefresh(win);
}

void w_gotoxy_puts(WINDOW *win, int x, int y, char *s) {
    mvwprintw(win, y, x, s);
    move(LINES - 1, 1);
    wrefresh(win);
}

void init_ui() {
    initscr();
    clear();
    if (!has_colors() || start_color() == ERR) {
        endwin();
        fprintf(stderr, "终端不支持颜色!\n");
        exit(1);
    }
    init_pair(1, COLOR_GREEN, COLOR_BLACK); //绿色
    init_pair(2, COLOR_RED, COLOR_BLACK);//红色
    init_pair(3, COLOR_WHITE, COLOR_BLACK);//白色
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);//黄色
    init_pair(5, COLOR_CYAN, COLOR_BLACK);//青色
    init_pair(6, COLOR_BLUE, COLOR_BLACK);//蓝色
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK); //洋红
    init_pair(8, COLOR_BLACK, COLOR_GREEN);
    init_pair(9, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(10, COLOR_BLACK, COLOR_RED);
    init_pair(11, COLOR_BLACK, COLOR_BLUE);
    init_pair(12, COLOR_BLACK, COLOR_YELLOW);

    message_win = create_newwin(MSG_WIDTH, MSG_HEIGHT, 2, 1);
    message_sub = subwin(message_win, MSG_HEIGHT - 2, MSG_WIDTH - 2, 2, 3);
    scrollok(message_sub, 1);
    info_win = create_newwin(INFO_WIDTH, MSG_HEIGHT, 2 + MSG_WIDTH, 1);
    info_sub = subwin(info_win, MSG_HEIGHT - 2, INFO_WIDTH - 2, 4, MSG_WIDTH + 4);
    //scrollok(info_sub, 1);
	input_win = create_newwin(INFO_WIDTH + MSG_WIDTH, INPUT_HEIGHT, 2, MSG_HEIGHT + 1);
    input_sub = subwin(input_win, MSG_HEIGHT + 1, MSG_WIDTH + INFO_WIDTH - 2, MSG_HEIGHT + 1, 4);

    wrefresh(message_win);
    wrefresh(info_win);
    wrefresh(input_win);
}


void show_message(WINDOW *win, struct ChatMsg *msg, int type) {
    time_t time_now = time(NULL);
    struct tm* tm = localtime(&time_now);
    char timestr[20] = {0};
    sprintf(timestr, "%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (type == 1) {
        strcpy(msg->name, "Server Info");
    } 
	strcat(msg->name, ": ");

    if (msgnum <= MSG_HEIGHT - 3) {
        if (type == 1) wattron(win, COLOR_PAIR(4));
		else if(type == -1) wattron(win, COLOR_PAIR(2));
		else wattron(win, COLOR_PAIR(5));
        w_gotoxy_puts(win, 1, msgnum, timestr);
        w_gotoxy_puts(win, 10, msgnum, msg->name);
        wattron(win, COLOR_PAIR(3));
        w_gotoxy_puts(win, 10 + strlen(msg->name), msgnum, msg->msg);
        msgnum++;
    } else {
        msgnum = MSG_HEIGHT - 3;
        scroll(win);
        if (type == 1) wattron(win, COLOR_PAIR(4));
		else if(type == -1) wattron(win, COLOR_PAIR(2));
		else wattron(win, COLOR_PAIR(5));
        w_gotoxy_puts(win, 1, msgnum, timestr);
        w_gotoxy_puts(win, 10, msgnum, msg->name);
        wattron(win, COLOR_PAIR(3));
        w_gotoxy_puts(win, 10 + strlen(msg->name), msgnum, msg->msg);
        msgnum++;
    }
    wrefresh(win);
    wrefresh(message_win);
}

void show_info(WINDOW *win, struct LogRequest *request) {
	char tmp[20];
	strcpy(tmp, "Logged in.");
	wattron(win, COLOR_PAIR(1));
	w_gotoxy_puts(win, 6, 4, tmp);
	strcpy(tmp, "User name:");
	wattron(win, COLOR_PAIR(7));
	w_gotoxy_puts(win, 6, 6, tmp);
	wattron(win, COLOR_PAIR(3));
	w_gotoxy_puts(win, 6, 7, request->name);
	if (request->team) {
		strcpy(tmp, "BLUE TEAM");
		wattron(win, COLOR_PAIR(6));
		w_gotoxy_puts(win, 6, 9, tmp);
	} else {
		strcpy(tmp, "RED TEAM");
		wattron(win, COLOR_PAIR(2));
		w_gotoxy_puts(win, 6, 9, tmp);
	}
	wrefresh(win);
    wrefresh(info_win);
}
