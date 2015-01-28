//
//  ScottishFold.h
//  LazyCat
//
//  Created by 李秦琦 on 14/12/19.
//  Copyright (c) 2014年 李秦琦. All rights reserved.
//
//  Server tool for LazyCat online auction system.

#ifndef __LazyCat__ScottishFold__
#define __LazyCat__ScottishFold__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define CMDSIZE 50
#define COUNTMAX 300

/* 用户手册 */
char Manual[] = "\
These commands are defined internally.  Type `/help' to see this manual.\n\
\n\
NAME\n\
ScottishFold - Server tool for the LazyCat online auction system.\n\
Author - 李秦琦 11300720096\n\
\n\
DESCRIPTION\n\
This is a server tool for auction managers.\n\
\n\
Using this tool is very easy. Invoke it from the prompt of your command interpreter as follows:\n\
\n\
shell> ./ScottishFold\n\
\n\
Read the document for more information about this kitten and services it provides.\n\
\n\
COMMANDS\n\
Note that all text commands must be first on line and end with \'\\n\'\n\
/?  			Synonym for `help'.\n\
/clear  		Clear screen.\n\
/msg 			Send messages to single or multiple users (default null).\n\
/list 			List current bidders.\n\
/kickout    	Kick out single or multiple users (default null).\n\
/openauction 	Begin a new auction and notify all online users current situation.\n\
/closeauction 	Close current auction and notify all online users with the result.\n\
/exit           Quit.\n\
";
int server_sockfd, client_sockfd;
int len;
int winnerID;   // 拍得者在 onboard_addr 中的位置
int clientN = 0;    // 登录客户端数
int priceHistory[COUNTMAX]; // 成交历史
struct sockaddr_in server_addr;   // 服务器网络地址结构体
struct sockaddr_in client_addr;   // 客户端网络地址结构体
struct sockaddr_in onboard_addr[100]; // 在线用户列表
int sin_size = sizeof(struct sockaddr_in);
char buf[BUFSIZ];  //数据传送的缓冲区
int currentAuction = 0; // 当前正在拍卖
int auctionCount = 0;   // 拍卖场数
int currentPrice;   // 当前出价
char * bidderHoldingHighestPrice = NULL; // 当前出价最高者


#endif /* defined(__LazyCat__ScottishFold__) */
