#include "work_thread.h"
#include <fcntl.h>

#define ARGC 20

//strart a thread
void thread_start(int c)
{
    pthread_t id;
    pthread_create(&id, NULL, work_thread, (void*)c);
}


void get_argv(char buff[], char * myargv[])
{
    char* saveptr;
    char* tmp;
    tmp  = strtok_r(buff, " ", &saveptr);
    int i = 0;
    while(1)
    {
        if( tmp == NULL)
        {
            break;
        }
        myargv[i++] = tmp;
        tmp =  strtok_r(NULL, " ",&saveptr);
    }
}

void send_file(int c, char* myargv[])
{
    if(myargv[1] == NULL)
    {
        send(c, "sorry:missing parament!\n", 25, 0);
        return;
    }
    int fd = open(myargv[1], O_RDONLY);
    if(fd == -1)
    {
        send(c, "sorry:this file does not exist!\n", 33, 0);
        return;
    }
    int file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char ser_status[128] = {0};
    sprintf(ser_status, "warn:document size:%2d,do you want to keep downloading? please reply yes or no?\n", file_size);
    send(c, ser_status, strlen(ser_status), 0);

    char cli_status[128] = {0};

    if(recv(c, cli_status, 127, 0) <= 0)
    {
        return;
    }
    printf("cli_status = %s\n",cli_status);
    if(strncmp(cli_status, "yes", 3) != 0)
    {
        return;
    }

    char read_buff[101] = {0};
    
    int num = 0;
    int read_size = 0;
    while(read_size < file_size)
    {
        memset(read_buff, 0, sizeof(read_buff));
        num = read(fd, read_buff, 100);
        if(num == -1)
        {
            return;
        }
        read_size = read_size + num;
        send(c, read_buff, num, 0);
    }
    close(fd);
    return;
}

int recv_file(int c, char* name)
{
    char cli_status[128] = {0};

    int recv_num = recv(c, cli_status, 127, 0 );
    if(recv_num <= 0)
    {
        send(c, "no", 2, 0);
        return -1;
    }

    printf("recv = %s",cli_status);
    fflush(stdout);

    char tmp[128] = {0};
    strcpy(tmp, cli_status);

    char* s= strtok(cli_status, ":");

    printf("s = %s\n", s);

    if(strncmp(s, "warn", 4) != 0)
    {
        
        printf("has return\n");
        return -1;
    }

   printf("%s",tmp);
   // printf("%s\n",cli_status);
   fflush(stdout);

    int fd = open(name, O_WRONLY|O_CREAT , 0600);
    if(fd == -1)
    {
        send(c, "no", 2, 0);
        return -1;
    }

    s = strtok(NULL, ":");
    s = strtok(NULL, ",");

    int file_size = 0;
    sscanf(s, "%d", &file_size);
    //printf("file_size = %d\n",file_size);
    
    printf("please input your choice:");
    fflush(stdout);

    char ser_status[10] = {0};
    fgets(ser_status, 10, stdin);
    send(c, ser_status, strlen(ser_status), 0);

    if(strncmp(ser_status, "yes", 3) != 0)
    {
        return -1;
    }
    char read_buff[101] = {0};
    int num = 0;
    int read_size = 0;
    while(read_size < file_size)
    {
        num = recv(c, read_buff, 100, 0);
        if(num == -1)
        {
            return -1;
        }
        read_size = read_size + num;
        write(fd, read_buff, num);
    }
    close(fd);
    printf("sucessful:file eceived!\n");
    return 0;
}

//worker thread
void* work_thread(void* arg)
{
    int c = (int)arg;
    //test
    while(1)
    {
        char buff[128] = {0};
        int n = recv(c, buff, 127, 0);
        if(n <= 0)
        {
            close(c);
            send(c, "no", 2, 0);
            printf("one client over!\n");
            break;
        }
        else
        {
            if(strncmp(buff, "uploading", 9) == 0)
            {
                send(c, "ok", 2, 0);
            }
        }

        char* myargv[ARGC] = {0};
        get_argv(buff, myargv);

        if(strcmp(myargv[0], "download") == 0)
        {
            send_file(c, myargv);
        }
        else if(strncmp(myargv[0], "uploading", 9) == 0)
        {
            recv_file(c, myargv[1]);
        }
        else
        {
            int pipefd[2];
            if( pipe(pipefd) < 0)
            {
                printf("sorry:pipe create error!\n");
                continue ;
            }
   
            pid_t pid = fork();

            if( pid == 0)
            {
                dup2(pipefd[1], 1);
                dup2(pipefd[1], 2);
            
                int exe = execvp(myargv[0], myargv);
                perror("sorry:(execv error)");
                exit(0);
            }
            else
            {
                close(pipefd[1]);
                wait(NULL);

                char r_buff[128] = {0};
                strcpy(r_buff,"RECIVE:\n");
                read(pipefd[0], r_buff+strlen(r_buff), 127);

                if(strcmp(r_buff,"RECIVE:\n") == 0)
                {
                    strcat(r_buff,"successful:order has been executed!\n");
                }
                
               send(c, r_buff, strlen(r_buff), 0);
            }
        }
    }
}

