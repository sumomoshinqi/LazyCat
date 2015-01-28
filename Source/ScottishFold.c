//
//  ScottishFold.c
//  LazyCat
//
//  Created by 李秦琦 on 14/12/19.
//  Copyright (c) 2014年 李秦琦. All rights reserved.
//
//  Server tool for LazyCat online auction system.

#include "ScottishFold.h"

void addUser (char username[])
{
    int i;
    onboard_addr[clientN] = client_addr;

    printf("\nNew client from %s:\n", inet_ntoa(client_addr.sin_addr));
    buf[len]='\0';
    printf("Name: ");
    for (i = 1; i < strlen(buf); i++)
        putchar(buf[i]);
    putchar('\n');
    
    /* 添加 biddername bidderID 到 userInfo.LazyCat */
    char toShell0[50], toShell1[50], toShell2[50];
    system("date +%Y%m%d%H%M%S >> userInfo.LazyCat");   // 为当前新用户分配一个 BidderID
    /* 纪录新用户的信息：IP, Name */
    sprintf(toShell0, "echo %d >> userInfo.LazyCat", clientN);
    system(toShell0);
    sprintf(toShell1, "echo %s >> userInfo.LazyCat", inet_ntoa(client_addr.sin_addr));
    system(toShell1);
    sprintf(toShell2, "echo %s >> userInfo.LazyCat", username);
    system(toShell2);
    memset(buf, 0, BUFSIZ);
    sprintf(buf, "Success");
    /* 反馈成功添加新用户 */
    if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("sendto failed");
    }
    clientN++;  // 更新客户数
    
    /* 如果当前拍卖正在进行，通知迟到用户 */
    if (currentAuction)
    {
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "\nCurrent auction is running. You're late!\nType `/refresh' to see current auction state.\n");
        if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }

    }
    
    return;
}

void refreshState ()
{
    if (!currentAuction)
    {
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "\nCurrently no auction is running.\n");
        if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
        return;
    }
    char line[100];
    memset(buf, 0, sizeof(buf));
    FILE * fp = fopen("auctionInfo.LazyCat", "r");
    while(fgets(line, sizeof(line), fp))
    {
        if(sendto(server_sockfd, line, strlen(line), 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
    }
    fclose(fp);
}

void newBid()
{
    memset(buf, 0, sizeof(buf));
    /* 当前无拍卖，竞价失败 */
    if (!currentAuction)
    {
        sprintf(buf, "Currently no auction running. Bidding failed.\n");
        if(sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
        return;
    }
    
    char toShell[100];
    char line[100];
    char * bidderIP = inet_ntoa(client_addr.sin_addr);  // 获取竞价者 IP
    char tmpArray[100];
    int i;
    FILE * fp;
    fp = fopen("userInfo.LazyCat","r");
    /* 在 userInfo.LazyCat 中查找该竞价者 */
    while(!feof(fp))
    {
        fgets(line, sizeof(line), fp);
        if (strncmp(line, bidderIP, strlen(bidderIP)) == 0)
        {
            fgets(line, sizeof(line), fp);  // 获取竞价者名称
            line[strlen(line)-1] = '\0';
            /* 在 auctionInfo.LazyCat 末尾添加本次竞价信息 */
            sprintf(toShell, "echo '$%d by %s' >> auctionInfo.LazyCat", currentPrice + 10, line);
            system(toShell);
            printf("\nNew bid : %d by %s from %s\n", currentPrice + 10, line, bidderIP);
            currentPrice += 10;
            fclose(fp);
            /* 通知竞价者竞价成功 */
            sprintf(buf, "Success! You're now holding the highest price.\n");
            if(sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
            {
                perror("sendto failed");
            }
            /* 在 onboard_addr 中查找该竞价者 */
            for (i = 0; i < clientN; i++)
            {
                memset(tmpArray, 0, sizeof(tmpArray));
                sprintf(tmpArray, "%s", inet_ntoa(onboard_addr[i].sin_addr));
                if (strncmp(tmpArray, inet_ntoa(client_addr.sin_addr), strlen(tmpArray)) == 0)
                    break;
            }
            /* 储存目前为止出价最高者 */
            winnerID = i;
            bidderHoldingHighestPrice = line;
            return;
        }
    }
    /* 查找失败，该竞价者非法，可能已经被踢出 */
    sprintf(buf, "You're not on the bidder list.\nBidding failed.\n");
    if(sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("sendto failed");
    }
    fclose(fp);
}           //                  \(0.0)/ \(^_^)/ \(-.-)/ \(*_*)/

void sendLeaveCode()
{
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "Uoyj9gJcuDBfEpcFKq9htFGW");   // code for leaving
    /* 发送 leaving code 使客户端下线 */
    sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr));
}

int holdingHighestPrice (char bidderName[])
{
    if (strcmp(bidderHoldingHighestPrice, bidderName) == 0)
        return 1;
    else
        return 0;
}

void bidderLeave()
{
    int i, j;
    char toShell[100];
    char line[100];
    char tmpArray[100];
    char * bidderIP = inet_ntoa(client_addr.sin_addr);
    FILE * fp;
    fp = fopen("userInfo.LazyCat","r");
    while(!feof(fp))
    {
        /* 在 userinfo.LazyCat 中查找准备离开的竞拍者 */
        fgets(line, sizeof(line), fp);
        if (strncmp(line, bidderIP, strlen(bidderIP)) == 0)
        {
            fgets(line, sizeof(line), fp);
            fclose(fp);
            line[strlen(line)-1] = '\0';
            /* 如果该竞拍者为最终出价者则不得离开 */
            if (holdingHighestPrice (line) && currentAuction)
            {
                memset(buf, 0, BUFSIZ);
                sprintf(buf, "\nCannot leave because you're holding the highest price.\n");
                /* 告知该竞拍者不得离开 */
                if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & client_addr, sizeof(struct sockaddr)) < 0)
                {
                    perror("sendto failed");
                }
                return;
            }
            /* 将 userInfo.LazyCat 中该用户改为离开状态 */
            sprintf(toShell, "sed 's/%s/%s [Leaved]/' < userInfo.LazyCat > userInfo.tmp", line, line);
            /* 执行 sed 命令来完成替换操作 */
            system(toShell);
            /* 删除中间文件 */
            system("mv userInfo.tmp userInfo.LazyCat");
            /* 利用 IP 地址在 onboard_addr 中查找该竞拍者的位置 */
            for (i = 0; i < clientN; i++)
            {
                memset(tmpArray, 0, sizeof(tmpArray));
                sprintf(tmpArray, "%s", inet_ntoa(onboard_addr[i].sin_addr));
                if (strncmp(tmpArray, inet_ntoa(client_addr.sin_addr), strlen(tmpArray)))
                    break;
            }
            /* 删去该竞拍者 */
            for (j = i; i < clientN; i++)
            {
                onboard_addr[j] = onboard_addr[j+1];
            }
            clientN--;
            sendLeaveCode();
            return;
        }
    }
    fclose(fp);
    /* 发送下线信号让该竞拍者的客户端下线 */
    sendLeaveCode();
}

void * listening (void * args)
{
    while (1)
    {
        /* 接收客户端的数据并将其发送给客户端--recvfrom是无连接的 */
        if((len = recvfrom (server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & client_addr, & sin_size)) < 0)
        {
            perror("recvfrom");
        }
        
        switch (buf[0])
        {
            /* 通过读取消息第一位的类型标识符来调用相应的模块处理客户端请求 */
            case '0':
                addUser(&buf[1]);   // 添加新用户
                break;
            case '1':
                refreshState(); // 客户端请求刷新拍卖状态
                break;
            case '2':
                newBid();   // 处理竞价
                break;
            case '3':
                bidderLeave();  // 客户端请求退出
                break;
            default:
                break;
        }
    }
    return NULL;
}

void listBidders()
{
    printf("\nCurrent online bidders:\n");
    system("python listBidders.py");    // 调用 listBidders.py
}

void sendMessage ()
{
    int Num = -1;
    char nameList[80];
    char userID[14];
    char line[50];
    getchar();
    /* 获取接收客户名单 */
    printf("Enter BidderIDs for message receivers: (Hit `Enter' when finish)\n");
    scanf("%[^\n]", nameList);
    getchar();
    /* 输入消息 */
    printf("Enter message to send:\n");
    char msg[100];
    scanf("%[^\n]", msg);   // 跳过空格
    getchar();
    sprintf(msg, "%s\n", msg);  // 加回尾部换行符
    printf("Sending ... \n");
    int i = 0, j;
    /* 对全部接收端重复以下过程 */
    while (nameList[i] != '\0')
    {
        /* 在 nameList 中获取每一个 ID */
        if (nameList[i] != ' ')
        {
            for (j = 0; j < 14; j++)
                userID[j] = nameList[i+j];
            userID[j] = '\0';
            FILE * fp;
            fp = fopen("userInfo.LazyCat","r");
            while(!feof(fp))
            {
                fgets(line, sizeof(line), fp);
                /* 找到该用户在 onboard_addr 中的位置 */
                if (strncmp(line, userID, 14) == 0)
                {
                    fscanf(fp, "%d", & Num);
                    break;
                }
            }
            fclose(fp);
            /* 发送消息 */
            if(sendto(server_sockfd, msg, strlen(msg), 0, (struct sockaddr *) & onboard_addr[Num], sizeof(struct sockaddr)) < 0)
            {
                perror("sendto failed");
            }
            i += 14;
            continue;
        }
        i++;
    }
    printf("Done"); // 消息发送完成
}

void kickoutBidders()
{
    char kickoutID[15], line[50];
    int shortID = -1, i;
    struct sockaddr_in tmpaddr;
    printf("Enter ID to kickout bidder :"); // 输入 Bidder ID 来踢出竞拍者
    scanf("%s", kickoutID);
    FILE * fp;
    fp = fopen("userInfo.LazyCat","r");
    /* 在 userInfo.LazyCat 中找到该用户 */
    while(!feof(fp))
    {
        fgets(line, sizeof(line), fp);
        if (strncmp(line, kickoutID, 14) == 0)
        {
            fscanf(fp, "%d", & shortID);
            break;
        }
    }
    fclose(fp);
    /* 从再现客户端列表中删去改客户端 */
    for (i = shortID; i < clientN-1; i++)
    {
        tmpaddr = onboard_addr[i];
        onboard_addr[i] = onboard_addr[i+1];
    }
    clientN--;  // 更新在线客户数
    system("python kickoutBidders.py"); // 从 userInfo.LazyCat 中删去改用户的条目
}

void openAuction()
{
    /* 必须完成当前拍卖才能新开一轮拍卖 */
    if (currentAuction)
    {
        printf("\nsCannot open a new auction.\nCurrent auction should be finished first.\n");
        return;
    }
    /* 检查是否有客户端登陆 */
    if (!clientN)
    {
        printf("\nNo bidder found.\n");
        return;
    }
    /* 生成纪录本次拍卖信息的临时文件 */
    system("touch auctionInfo.LazyCat");
    auctionCount++; // 拍卖常熟加一
    winnerID = -1;  // 初始化拍得者
    char toShell[100];  // Shell 命令暂存
    /* 向 auctionInfo.LazyCat 文件尾写入新拍卖的信息 */
    sprintf(toShell, "echo Auction [%d] starting. > auctionInfo.LazyCat", auctionCount);
    system(toShell);    // 执行 Bash 命令
    int i = 0;
    currentPrice = 100; // 起拍价
    memset(buf, 0, sizeof(buf));
    
    /* 通知所有登录用户新拍卖开始 */
    sprintf(buf, "Announcement : A new auction opened.\nStarting price $100\n");
    system("echo 'Starting price $100' >> auctionInfo.LazyCat");
    for (i = 0; i < clientN; i++)
    {
        if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & onboard_addr[i], sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
    }
    currentAuction = 1; // 拍卖启动
}

void startServer()
{
    printf("Server starting");
    clientN = 0;
    printf(" ...");
    system("touch serverIP.LazyCat");
    system("touch userInfo.LazyCat");
    
    printf(" ...");
    memset(& server_addr, 0, sizeof(server_addr)); // 数据初始化--清零
    server_addr.sin_family = AF_INET;   // 设置为IP通信
    server_addr.sin_addr.s_addr = INADDR_ANY; // 服务器IP地址--允许连接到所有本地地址上
    server_addr.sin_port = htons(8000);   // 服务器端口号
    
    /* 创建服务器端套接字--IPv4协议，面向无连接通信，UDP协议 */
    if((server_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Error: Can not create socket.");
        return;
    }
    
    printf(" ...");
    
    /* 将套接字绑定到服务器的网络地址上 */
    if (bind(server_sockfd,(struct sockaddr *)& server_addr,sizeof(struct sockaddr))<0)
    {
        perror("Error: Bind failed.");
        return;
    }
    
    printf(" ...");
    
    sin_size = sizeof(struct sockaddr_in);
    
    system("python getIP.py");  // 获取本机 IP
    printf(" [Done]\n");
}

void notifyWinner ()
{
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\n---------------\nDear %s,\nCongratulations! You win!\n---------------\n", bidderHoldingHighestPrice);
    /* 向拍得者发送通知 */
    if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & onboard_addr[winnerID], sizeof(struct sockaddr)) < 0)
    {
        perror("sendto failed");
    }
}

void abortiveAuction ()
{
    int i;
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\n---------------\nCurrent auction failed. No winner.\n---------------\n");
    for (i = 0; i < clientN; i++)
    {
        if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & onboard_addr[i], sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
    }
}

void closeAuction()
{
    if (winnerID == -1)
        abortiveAuction();  // 流拍
    int i, sum;
    float averagePrice, lowestPrice, highestPrice;
    priceHistory[auctionCount-1] = currentPrice;    // 纪录本轮成交价
    /* 计算历史平均成交价 */
    for (i = 0, sum = 0; i < auctionCount; i++)
        sum += priceHistory[i];
    averagePrice = sum / auctionCount;
    /* 计算历史最低成交价 */
    for (i = 1, lowestPrice = priceHistory[0]; i < auctionCount; i++)
    {
        if (priceHistory[i] < lowestPrice)
            lowestPrice = priceHistory[i];
    }
    /* 计算历史最高成交价 */
    for (i = 1, highestPrice = priceHistory[0]; i < auctionCount; i++)
    {
        if (priceHistory[i] > highestPrice)
            highestPrice = priceHistory[i];
    }
    /* 拍卖成功，向全体在线客户端发送拍卖成功信息和历史价格 */
    if (winnerID != -1)
    {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "\nSold!\n---------------\nHammer price: $%d\n---------------\n\nPrice history:\nLowest: $%f\nHighest: $%f\n---------------\nAverage: $%f\n---------------\n", currentPrice, lowestPrice, highestPrice, averagePrice);
        for (i = 0; i < clientN; i++)
        {
            if(sendto(server_sockfd, buf, BUFSIZ, 0, (struct sockaddr *) & onboard_addr[i], sizeof(struct sockaddr)) < 0)
            {
                perror("sendto failed");
            }
        }
        /* 通知拍得者 */
        notifyWinner ();
    }
    /* 本次拍卖结束，让客户端下线 */
    for (i = 0; i < clientN; i++)
    {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "Uoyj9gJcuDBfEpcFKq9htFGW");
        if(sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & onboard_addr[i], sizeof(struct sockaddr)) < 0)
        {
            perror("sendto failed");
        }
    }
    /* 删除临时文件 */
    system("rm auctionInfo.Lazycat");
    system("rm userInfo.Lazycat");
    system("touch userInfo.LazyCat");
    /* 变量还原 */
    clientN = 0;
    currentAuction = 0;
}

void exitScottishFold()
{
    int i;
    char command[10];
    /* 当前拍卖未完成，特殊情况立即退出 */
    if (currentAuction)
    {
        printf("Currently auction is running. Are you sure to exit? [Y/n]");
        scanf("%s", command);
        if(strcmp(command, "Y") == 0 || strcmp(command, "y") == 0)
            exit(0);    // 强制退出
        else
            return;
    }
    else
    {
        /* 强制让所有客户端离线 */
        printf("Force shutting down clients ... ");
        for (i = 0; i < clientN; i++)
        {
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "PaVk49AavrrxmuMxxADUjcr");    // 强制下线信号
            if(sendto(server_sockfd, buf, strlen(buf), 0, (struct sockaddr *) & onboard_addr[i], sizeof(struct sockaddr)) < 0)
            {
                perror("sendto failed");
            }
        }
        // 删除所有临时文件 */
        system("rm *.LazyCat");
        printf("Done\n.........\nBye");
        exit(0);
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
    if (strcmp(command, "/msg") == 0)
    {
        sendMessage();
        return;
    }
    if (strcmp(command, "/list") == 0)
    {
        listBidders();
        return;
    }
    if (strcmp(command, "/kickout") == 0)
    {
        kickoutBidders();
        return;
    }
    if (strcmp(command, "/openauction") == 0)
    {
        openAuction();
        return;
    }
    if (strcmp(command, "/closeauction") == 0)
    {
        closeAuction();
        return;
    }
    if (strcmp(command, "/exit") == 0)
    {
        exitScottishFold();
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
    startServer();  // 启动服务器
    pthread_t listeningT, readCMD;  // 用两个线程来负责监听和处理命令
    pthread_create(&listeningT, NULL,(void *)listening, NULL);
    pthread_create(&readCMD, NULL, (void *)readCommand, NULL);
    pthread_join(listeningT, NULL);
    pthread_join(readCMD, NULL);
    return 0;
}