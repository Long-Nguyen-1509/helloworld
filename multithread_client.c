#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

void* thread_proc(void *arg);

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect() failed");
        return 1;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
    char buf[256];
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);
        if (strncmp(buf, "exit", 4) == 0) 
            break;
    }
    close(client);
    return 0;
}

void* thread_proc(void *arg) 
{
    int client = *(int *)arg;
    char buf[256];
    while (1) 
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        printf("%s\n", buf);
    }
    close(client);
    return 0;
}