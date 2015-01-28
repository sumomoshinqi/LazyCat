//
//  Chinchilla.c
//  LazyCat
//
//  Created by 李秦琦 on 14/12/19.
//  Copyright (c) 2014年 李秦琦. All rights reserved.
//
//  Client tool for LazyCat online auction system.

#include "Chinchilla.h"

void makeBuf (int type, char content[])
{
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%d%s", type, content);
}

void startClient()
{
    printf("Client starting ... ...\n");
    printf("Server IP: %s\n", serverIP);
    printf("Setting up client ...");
    memset(& server_addr, 0, sizeof(server_addr)); // 数据初始化--清零
    server_addr.sin_family = AF_INET;   // 设置为IP通信
    server_addr.sin_addr.s_addr = inet_addr(serverIP); // 服务器IP地址
    server_addr.sin_port = htons(8000);   // 服务器端口号
    
    /*创建客户端套接字--IPv4协议，面向无连接通信，UDP协议*/
    if((client_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error: Can not create socket.");
        exit(EXIT_FAILURE);
    }
    printf(" ... ... [Done]\n");
}

void exitChinchilla()
{
    exit(0);
}

void * listening (void * args)
{
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        /* 接收来自服务器的消息 */
        if((len = recvfrom(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, &sin_size)) < 0)
        {
            perror("recvfrom");
        }
        /* 处理特殊信号 */
        if (strcmp(buf, "Uoyj9gJcuDBfEpcFKq9htFGW") == 0)
        {
            printf("\nleaving..."); // 拍卖完成
            exit(0);
        }
        if (strcmp(buf, "PaVk49AavrrxmuMxxADUjcr") ==0)
        {
            printf("\nServer down. Force quit.");   // 强制下线
            exit(0);
        }
        printf("\n------------------------\n%s", buf);
    }
    return NULL;
}

void clientLoggin()
{
    char userName[20];
    /* 登录名 */
    printf("Enter your name\n(Note: Other bidders won't see this):");
    scanf("%s", userName);
    makeBuf (0, userName);  // 创建发送用 buf，0 表示该消息为登录信息
    printf("Loggin as %s ...", userName);
    /* 发送登录请求 */
    if(sendto(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Loggin failed");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
    memset(buf, 0, sizeof(buf));
    /* 接收登录成功反馈 */
    if((len = recvfrom(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, &sin_size)) < 0)
    {
        perror("recvfrom");
    }
    puts(buf);  // 显示服务器反馈
    getchar();
}

void clientRefresh()
{
    printf("Requesting latest auction state...\n");
    makeBuf(1, NULL);   // 发送 refresh 请求，标识符为 1
    /* 发送请求 */
    if(sendto(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Refresh failed");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
}

void clientBid()
{
    printf("Trying to bid...\n");
    makeBuf(2, NULL);   // 发送 bid 请求，标识符为 2
    /* 发送请求 */
    if(sendto(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Bidding failed");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
}

void clientLeave()
{
    printf("Leaving ...\n");
    makeBuf(3, NULL);   // 发送 leave 请求，标识符为 3
    /* 发送请求 */
    if(sendto(client_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Leaving failed");
        close(client_sockfd);
        exit(EXIT_FAILURE);
    }
}

void exeCommand(char command[])
{
    if (command[0] != '/')
    {
        printf("Invalid format. Commands should start with `/\'\n");
        return;
    }
    if (strcmp(command, "/help") == 0 || strcmp(command, "/?") == 0)   //See manual
    {
        puts(Manual);
        return;
    }
    if (strcmp(command, "/clear") == 0)   //Clear
    {
        system("clear");
        return;
    }
    if (strcmp(command, "/refresh") == 0)
    {
        clientRefresh();
        return;
    }
    if (strcmp(command, "/bid") == 0)
    {
        clientBid();
        return;
    }
    if (strcmp(command, "/leave") == 0)
    {
        clientLeave();
        return;
    }
    printf("Command not found.\n");
    return;
}

void * readCommand (void * args)
{
    char command[CMDSIZE];
    while (1)
    {
    GETCMD:
        putchar('>');
        memset(command, 0, sizeof(command));
        scanf("%[^\n]", command);
        exeCommand(command);
        getchar();
    }
    return NULL;
}

int main()
{
    startClient();  // 启动客户端
    clientLoggin(); // 登录服务器
    pthread_t listeningT, readCMD;  // 用两个线程来负责监听和处理命令
    pthread_create(&listeningT, NULL,(void *)listening, NULL);
    pthread_create(&readCMD, NULL, (void *)readCommand, NULL);
    pthread_join(listeningT, NULL);
    pthread_join(readCMD, NULL);
    return 0;
}