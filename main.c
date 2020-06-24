#include "ftp.h"

extern int sockfd;
extern int LOGIN_STATUS;

int main()
{
    LOGIN_STATUS = 0;
    char cmd[10];
    memset(cmd, 0, sizeof cmd);

    printf("### login || pwd || ls || quit ### \n");
    while (true) {
        fflush(stdin);
        printf("ftp> ");
        scanf("%s", cmd);

        if (0 != cmd[(sizeof cmd) - 1]) {
            fprintf(stderr, ">| cmd format is Error! \n");
            exit(4);
        }

        if (1 == LOGIN_STATUS) {    // usr 登录状态为在线
            if (0 == strncmp(cmd, "login", 5))      // usr 已登录的情况下，不可以重复登录
            { fprintf(stderr, ">| Warning, usr already on-line! Don't repeat login!\n"); }   
            else if (0 == strncmp(cmd, "pwd", 3)) 
            { ftp_cmd_PWD(); }
            else if (0 == strncmp(cmd, "ls", 2)) 
            { ftp_cmd_LS(); }
            else if (0 == strncmp(cmd, "quit", 4)) 
            { ftp_cmd_QUIT(); break; }
            else 
            { fprintf(stderr, ">| %s: command not found\n", cmd); }
        }
        else {      // usr 登录状态为离线
            if (0 == strncmp(cmd, "login", 5)) 
            { ftp_LOGIN(); }
            else if (0 == strncmp(cmd, "quit", 4))
            { ftp_cmd_QUIT(); break; }
            else 
            { fprintf(stderr, ">| Warning, usr status -> off-line!\n"); }
        }

        memset(cmd, 0, sizeof cmd);
    }

    return 0;
}

