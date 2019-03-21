#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int recv_file(int sockfd, char* name)
{
    char buff[128] = {0};
    if(recv(sockfd, buff, 127, 0) <= 0)
    {
        return -1;
    }
    printf("%s",buff);

    char*s = strtok(buff, ":");

   // printf("s = %s\n", s);

    if(strcmp(s, "sorry") == 0)
    {
        return -1;
    }

    s = strtok(NULL, ":");
    s = strtok(NULL, ",");

    int file_size = 0;
    sscanf(s, "%d", &file_size);

    int fd = open(name, O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        send(sockfd, "no", 2, 0);
        return -1;
    }

    printf("please input your choice:");
    char reply_buff[5] = {0};
    fgets(reply_buff, 4, stdin);
    send(sockfd, reply_buff, strlen(reply_buff), 0);
   
    if(strncmp(reply_buff, "yes", 3) != 0)
     {
         return ;
     }

    int num = 0;
    char write_buff[101] = {0};
    int percent_size = 0;
    
    printf("\033[?25l");
    while(percent_size < file_size)
    {
        num =  recv(sockfd, write_buff, 101, 0);
        if(num == -1)
        {
            return -1;
        }
        write(fd, write_buff, num);

        percent_size = percent_size + num;
        float percent = percent_size * 100.00 / file_size;
        printf("downloading:%.2f%%\r",percent);
        fflush(stdout);
        memset(write_buff, 0, sizeof(write_buff));
    }
    close(fd);
    printf("\033[?25h");
    printf("sucessful:download completes!\n");
    return 0;
}

void send_file(int sockfd, char* myargv[])
{
    if(myargv[1] == NULL)
    {
        printf("sorry:miss paramenting!\n");
        send(sockfd, "sorry:\n",8,  0);
        return;
    }
    int fd = open(myargv[1], O_RDONLY);
    if(fd == -1)
    {
        printf("sorry1:this file not find\n");
        send(sockfd, "sorry1:\n",8,  0);
        return;
    }

    int file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char cli_status[128] = {0};
    sprintf(cli_status, "warn:document size:%2d,whether to recive? please yes or no.\n",file_size);
    send(sockfd, cli_status, strlen(cli_status), 0);

    char ser_status[128] = {0};
    if(recv(sockfd, ser_status, 127, 0) <= 0)
    {
        printf("sorry:receive abnormal\n");
        return;
    }
    else if(strncmp(ser_status, "yes", 3) != 0)
    {
        printf("ser_status = %s\n",ser_status);
        printf("sorry:fail to upload\n");
        return;
    }
    else
    {
        ser_status[4] = '0';
        printf("ser_status = %s",ser_status);
        fflush(stdout);

        char read_buff[101] = {0};
        int num = 0;
        int read_size = 0;
        printf("\033[?25l");
        while(read_size < file_size)
        {
            num = read(fd, read_buff, 100);
            if(num == -1)
            {
                return ;
            }
            read_size = read_size + num;
            float percent = read_size *100.00 / file_size;
            send(sockfd, read_buff, strlen(read_buff), 0);
            printf("uploading:%.2f%%\r",percent);
            fflush(stdout);
        }
        close(fd);
        printf("\033[?25h");
        return;
    }
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int res = connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    assert(res != -1);

    while(1)
    {
        char buff[128] = {0};
        printf("[please input order:]");
        
        fgets(buff, 128, stdin);

        if( strncmp(buff, "end", 3) == 0)
        {
            break;
        }

        buff[strlen(buff) - 1] = 0;
        if(buff[0] == 0)
        {
            continue;
        }

        char tmp[128] = {0};
        strcpy(tmp, buff);

        char* myargv[10] = {0};
        char* s = strtok(tmp, " ");

        int i = 0;
        while(s != NULL)
        {
            myargv[i++] = s;
            s = strtok(NULL, " ");
        }
        
        if(strcmp(myargv[0], "download") == 0)
        {
            send(sockfd, buff, strlen(buff), 0);
            recv_file(sockfd, myargv[1]);
        }
        else if(strcmp(myargv[0], "uploading") == 0)
        {
            send(sockfd, buff, strlen(buff), 0);
            char if_ok[5] = {0};
            recv(sockfd,if_ok, 4, 0 );
            if(strcmp(if_ok, "ok") != 0)
            {
                continue;
            }
            send_file(sockfd, myargv);
        }
        else
        {
            send(sockfd, buff, strlen(buff), 0);
            char read_buff[1024] = {0};
            recv(sockfd,read_buff, 1023, 0);
            printf("%s",read_buff);
        }
        
    }
    close(sockfd);
    exit(0);
}
