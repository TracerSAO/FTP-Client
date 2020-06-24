#ifndef FTP_H
#define FTP_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>     // -> struct sockaddr_in addr;
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SERV_PORT 21
#define MAXSIZE 256
#define HOST_SIZE 18    // 限定 usr 输入 IP 地址最大容量 -> 4 * 4 + 3 + ('\0') = 18

char *_host;            // 存储 usr IP 地址
int sockfd;             // 套接字 ID: 命令控制线路
int sockfd_DATAPORT;    // 套接字 ID: 数据传输线路
int LOGIN_STATUS;       // usr 登录状态 { 0 -> 离线 | 1 -> 登录}
int DATA_PORT;          // 获取 service 在 被动模式下开启的第二端口，即：数据端口 
                        // 转换模式后，service 会返回 227 entering passive mode (h1,h2,h3,h4,p1,p2) -> data_PORT = p1 * 256 + p2 

// LOGIN -> 登录
void ftp_LOGIN();
void ftp_Switch_MODE();             // ftp_LOGIN() -> call
void ftp_Connect_DATAPORT();        // ftp_LOGIN() -> call

// command -> 命令类
void ftp_cmd_LS();
void ftp_cmd_PWD();
void ftp_cmd_UP();
void ftp_cmd_DOWN();
void ftp_cmd_MKDIR();
void ftp_cmd_HELP();
void ftp_cmd_QUIT();

#endif // ftp.h
