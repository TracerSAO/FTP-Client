#include "ftp.h"

extern char *_host;
extern int sockfd;
extern int sockfd_DATAPORT;
extern int LOGIN_STATUS;
extern int DATA_PORT;

void ftp_LOGIN()
{
    // 初始化 client 连接信息
    struct sockaddr_in serv_addr;       // 记录 service 端口 info
    typedef struct sockaddr SA;
    char send_buf[MAXSIZE], recv_buf[MAXSIZE];      // send & recv 与服务器交互的信息
    char usrName[MAXSIZE], usrPasswd[MAXSIZE];      // 非匿名方式登录 ftp-service

    char *host_ = NULL;
    _host = (char*) malloc(sizeof(char) * HOST_SIZE);
    // struct hostent *host;        // 暂不提供较复杂的域名解析

    // 获取 server IP 地址
    memset(_host, 0, HOST_SIZE);
    printf("*| Hostname: ");
    scanf("%s", _host);
    if (0 != _host[17]) {
        fprintf(stderr, ">| hostname is error! \n");
        memset(_host, 0, HOST_SIZE);
        exit(2);
    }  

    // 初始化 service 端口 info
    bzero(&serv_addr, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);               // 暂时只默认提供 21 端口
    serv_addr.sin_addr.s_addr = inet_addr(_host);


    // 创建 socket() 套接字
    if (0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
        printf(">| Socket Error! \n");
        exit(1);
    }
    printf("*| Socket creation successfully... \n");

    // 连接 service 端口
    if (0 > (connect(sockfd, (SA*)&serv_addr, sizeof serv_addr))) {
        printf(">| connect Error! \n");
        exit(1);
    }
    printf("*| connect -> %s::21 succeed... \n", _host);
    recv(sockfd, recv_buf, MAXSIZE, 0);
    printf("*| %s \n", recv_buf);

    
    // 登录 service
    /* 确认 usrName */
    memset(send_buf, 0, sizeof send_buf);
    memset(recv_buf, 0, sizeof recv_buf);
    memset(usrName,  0, sizeof usrName);
    fflush(stdin);
    printf("*| Usr name(maxsize < 256): ");
    scanf("%s", usrName);
    if (0 != usrName[MAXSIZE - 1]) {
        fprintf(stderr, "*| usr name is Error! \n");
        exit(3);
    }

    sprintf(send_buf, "USER %s\r\n", usrName);                      // 注意字符大小引发的错误
    if (-1 == send(sockfd, send_buf, (int)strlen(send_buf), 0)) {
        printf(">| send(name)  Error! \n");
        exit(3);
    }

    recv(sockfd, recv_buf, MAXSIZE, 0);
    if (0 == strncmp(recv_buf, "331", 3)) { 
        printf("*| usr Name -> OK \n");
        printf("*| %s \n", recv_buf);
    }
    else {
        printf(">| usr Name -> Error \n");
        fprintf(stderr, "%s \n", recv_buf);
        exit(3);
    }


    /* 确认 usrPasswd */
    memset(send_buf, 0, sizeof send_buf);
    memset(recv_buf, 0, sizeof recv_buf);
    memset(usrPasswd, 0, sizeof usrPasswd);
    fflush(stdin);
    printf("*| Passwd(maxsize < 256): ");
    scanf("%s", usrPasswd);
    if (0 != usrPasswd[MAXSIZE - 1]) {
        fprintf(stderr, ">| usr passward is Error! \n");
        exit(3);
    }

    sprintf(send_buf, "PASS %s\r\n", usrPasswd);                    // 注意字符大小引发的错误
    if (-1 == send(sockfd, send_buf, (int)strlen(send_buf), 0)) {
        printf(">| passwd send is wrong \n");
        exit(3);
    }

    recv(sockfd, recv_buf, MAXSIZE, 0);
    if (0 == strncmp(recv_buf, "230", 3)) { 
        printf("*| user passwd -> OK \n");
        printf("*| %s \n", recv_buf);

        LOGIN_STATUS = 1;   // 设定 usr 在线状态
	ftp_Switch_MODE();  // 设定 service 为 PASV mode
        ftp_Connect_DATAPORT(); // 连接 service DATA_PORT
    }
    else {
        printf(">| user passwd is Error! \n");
        exit(3);
    }
}

void ftp_Switch_MODE()
{
    char send_buf[MAXSIZE], recv_buf[MAXSIZE];                  // send & recv 与服务器交互的信息

    memset(send_buf, 0, sizeof send_buf);
    memset(recv_buf, 0, sizeof recv_buf);
    sprintf(send_buf, "PASV\r\n");

    if (-1 == send(sockfd, send_buf, (int)strlen(send_buf), 0)) {
        printf(">| Switch_Mode, send is Error!\n");
    }
    printf("*| send -> succeed\n");

    recv(sockfd, recv_buf, MAXSIZE, 0);
    if (0 == strncmp(recv_buf, "227", 3)) {
        printf("*| Switch Mode -> succeed, service's mode \"PASV\"\n");
        printf("*| %s\n", recv_buf);
    }
    else {
        printf(">| Switch Mode -> Error!\n");
        printf(">| %s\n", recv_buf);
        exit(5);
    }

    /* 解析 service 返回的 info -> 获取 NO.2 port */
    // format: 227 entering passive mode (h1,h2,h3,h4,p1,p2)
    int temp_i = strlen(recv_buf) - 1;
    int temp_R = 0;                 // 确定右括号的位置
    int temp_a = 0, temp_b = 0;     // 确定后两个 ',' 的位置
    int count = 0;

    while (true) {
        if (')' == recv_buf[temp_i])
            temp_R = temp_i;
        else if (',' == recv_buf[temp_i]) {
            if (0 == count) {
                temp_b = temp_i;
                count++;
            } 
            else if (1 == count) {
                temp_a = temp_i;
                break;
            }
        }

        temp_i--;
    }

    char temp_chA[4], temp_chB[4];
    memset(temp_chA, 0, 4);
    memset(temp_chB, 0, 4);
    strncat(temp_chA, &recv_buf[temp_a + 1], temp_b - temp_a - 1);
    strncat(temp_chB, &recv_buf[temp_b + 1], temp_R - temp_b - 1);

    temp_a = atoi(temp_chA);
    temp_b = atoi(temp_chB);
    DATA_PORT = temp_a * 256 + temp_b;
    printf("*| DATA_PORT get successfully...\n");
}

void ftp_Connect_DATAPORT()
{
    struct sockaddr_in dataPort_addr;       // 记录 service 端口 Info
    typedef struct sockaddr SA;
    char recv_buf[MAXSIZE];

    memset(recv_buf, 0, MAXSIZE);

    bzero(&dataPort_addr, sizeof dataPort_addr);
    dataPort_addr.sin_family = AF_INET;             // 支持 IPV4 格式
    dataPort_addr.sin_port = htons(DATA_PORT);      // 设定端口为 “数据传输端口”
    dataPort_addr.sin_addr.s_addr = inet_addr(_host);

    // 初始化 Socket() 套接字 -> DATA_PORT
    if (0 > (sockfd_DATAPORT = socket(AF_INET, SOCK_STREAM, 0))) {
        printf(">| DATA_PORT Socket!\n ");
        exit(1);
    }
    printf("*| DATA_PORT Socket creation successfully...\n");

    // connect service DATA_PORT
    if (0 > connect(sockfd_DATAPORT, (SA*)&dataPort_addr, sizeof dataPort_addr)) {
        printf(">| DATA_PORT connect Error!\n");
        exit(1);
    }
    printf("*| connect -> %s::%d succeed...\n", _host, DATA_PORT);

    //sleep(100);
    //if (0 > recv(sockfd_DATAPORT, recv_buf, MAXSIZE, 0)) {
    	//fprintf(stderr, ">| recv is Error!\n");
	//exit(1);
    //}
    //printf("*| %s \n", recv_buf);

}

void ftp_cmd_LS()
{
    char send_buf[MAXSIZE], recv_buf[MAXSIZE];          // send & recv 与服务器交互的信息
    char recv_line[1024];				// recv 服务器 data_PORT 信息

    memset(send_buf, 0, sizeof send_buf);
    memset(recv_buf, 0, sizeof recv_buf);
	memset(recv_line, 0, sizeof recv_line);

    sprintf(send_buf, "LIST\r\n");
    if (-1 == send(sockfd, send_buf, (int)strlen(send_buf), 0)) {
        printf(">| LIST, send is Error!\n");
        exit(1);
    }
    printf("*| LIST, send -> succeed\n");

    recv(sockfd, recv_buf, MAXSIZE, 0);
	if (0 != strncmp(recv_buf, "150", 3)) {
		printf("*| %s\n", recv_buf);		
		fprintf(stderr, ">| LIST, recv -> Error!\n");    	
		exit(4);	
	}
	printf("*| LIST, recv -> succeed\n");
	printf("*| %s\n", recv_buf);
    recv(sockfd_DATAPORT, recv_line, sizeof recv_line, 0);
	printf("%s\n", recv_line);

}

void ftp_cmd_PWD()
{
    char send_buf[MAXSIZE], recv_buf[MAXSIZE];
	memset(send_buf, 0, MAXSIZE);
	memset(recv_buf, 0, MAXSIZE);
	
	sprintf(send_buf, "PWD\r\n");

	if (-1 == send(sockfd, send_buf, (int)strlen(send_buf), 0)) {
			fprintf(stderr, ">| PWD, send is Error!\n");
			exit(1);
	}
	printf("*| PWD, send -> succeed\n");
	
	recv(sockfd, recv_buf, (int)strlen(recv_buf), 0);
	if (0 != strncmp(recv_buf, "226", 3)) {
		printf("*| %s\n", recv_buf);
		fprintf(stderr, ">| PWD, recv -> Error!\n");
		exit(4);
	}
	printf("*| PWD, recv -> succeed\n");
	printf("*| %s\n", recv_buf);
	
	int temp_L = 0, temp_R = 0, temp_i = 0;
	int count = 0;	
	while (true) {
		if ('"' == recv_buf[temp_i]) {
			if (0 == count) {
				temp_L = temp_i;
				count++;			
			}
			else if (1 == count) {
				temp_R = temp_i;
				break;			
			}
		}
		temp_i ++;
	}

	char temp_ch[MAXSIZE];
	memset(temp_ch, 0, MAXSIZE);
	strncat(temp_ch, &recv_buf[temp_L + 1], temp_R - temp_L - 1);
	printf("*| PWD: %s\n", temp_ch);

}

void ftp_cmd_UP()
{
    printf("...\n");

}

void ftp_cmd_DOWN()
{
    printf("...\n");

}

void ftp_cmd_MKDIR()
{
    printf("...\n");

}

void ftp_cmd_HELP()
{
    printf("...\n");

}

void ftp_cmd_QUIT()
{
    if (0 == LOGIN_STATUS)
        printf("<0>...\n");
    else {
        printf("<1>...\n");
        close(sockfd);
		close(sockfd_DATAPORT);
        printf("connection break... \n");
    }
}
