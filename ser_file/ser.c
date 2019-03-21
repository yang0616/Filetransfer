#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "work_thread.h"

#define PORT 6000
#define IPSTR "127.0.0.1"
#define LIS_MAX 5

// create a socket
int create_socket();

//accept link function
int accept_fun(int sockfd);

int main()
{
    int sockfd = create_socket();
    if( sockfd == -1)
    {
        printf("sorry:create sockfd error!\n");
        exit(0);
    }
    else
    {
        printf("successful:the socket is created!\n");
    }

    while(1)
    {
        int c = accept_fun(sockfd);
        if(c == -1)//check if the connection is successful
        {
            printf("sorry:accept error!\n");
            continue;
        }
        else//print descriptor
        {
            printf("one connection(deasciption = %d) was successfully established!\n",c);
        }
       thread_start(c);
    }
    exit(0);
}

int accept_fun(int sockfd)
{
    struct sockaddr_in caddr;
    int len = sizeof(caddr);
    int c = accept(sockfd, (struct sockaddr*)&caddr, &len);

    return c;
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd == -1)
    {
        return -1;
    }

    struct sockaddr_in saddr, caddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr(IPSTR);

    int res = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(res == -1)
    {
        perror("bind error!");
        return -1;
    }

    listen(sockfd, LIS_MAX);

    return sockfd;
}

