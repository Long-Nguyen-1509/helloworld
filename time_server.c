
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

time_t rawtime;
struct tm *info;

int num_clients = 0;

void signalHandler(int signo) {
    int stat;
    pid_t pid = wait(&stat);
    printf("Child %d terminated.\n", pid);
    return;
}

void date_process(char *format, char *buf)
{
    time( &rawtime );
    info = localtime( &rawtime );
    strftime(buf, 32, format, info);
} 

void process_request(int client, char *buf)
{
    char cmd[32], format[32];
    int ret = sscanf(buf, "%s %s", cmd, format);
    if (ret == 2)
    {   
        if (strcmp(cmd, "GET_TIME") == 0)
        {
            if (strcmp(format, "dd/mm/yyyy") == 0)
            {
                date_process("%d/%m/%Y", buf);
                send(client, buf, strlen(buf), 0);
            }
            else if (strcmp(format, "dd/mm/yy") == 0)
            {
                date_process("%d/%m/%y", buf);
                send(client, buf, strlen(buf), 0);
            }
            else if (strcmp(format, "mm/dd/yyyy") == 0)
            {
                date_process("%m/%d/%Y", buf);
                send(client, buf, strlen(buf), 0);
            }
            else if (strcmp(format, "mm/dd/yy") == 0)
            {
                date_process("%m/%d/%y", buf);
                send(client, buf, strlen(buf), 0);
            }
            else
            {
                char *msg = "Sai format Hay nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }  
        }
        else
        {
            char *msg = "Sai lenh. Hay nhap lai.\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        char *msg = "Sai tham so. Hay nhap lai.\n";
        send(client, msg, strlen(msg), 0);
    } 
}

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);
    while (1)
    {
        printf("Waiting for new client...\n");
        int client = accept(listener, NULL, NULL);
        if (fork() == 0)
        {
            // Tien trinh con
            close(listener);

            char buf[256];

            // Xu li request
            if(num_clients < 64)
            {   
                printf("New client connected: %d\n", client);
                fflush(stdout);
                num_clients++;
            }
            else
            {
                // Dang co qua nhieu ket noi
                char *msg = "He thong qua tai, vui long thu lai sau,\n";
                send(client, msg, strlen(msg), 0);
                close(client);
            }
            while(1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    break;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", client, buf);
                    fflush(stdout);
                    process_request(client, buf);
                }
            }
            close(client);
            num_clients--;
            exit(0);
        }
        //Tien trinh cha
        close(client);
    }
    close(listener);
    return 0;
}


                        
                    