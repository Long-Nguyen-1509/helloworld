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
#include <stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void* thread_proc(void *arg);

int main(){
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

    int num_threads = 8;
    pthread_t thread_id;

    for (int i = 0; i < num_threads; i++) 
    {
        int ret = pthread_create(&thread_id, NULL, thread_proc, &listener);
        if (ret != 0)
        {
            printf("Could not create new thread.\n");
            sched_yield();
        }
        pthread_join(thread_id, NULL);
        return 0;
    }
}

void* thread_proc(void *arg) {
    int listener = *(int *)arg;
    char buf[256];
    while (1) {
        // Chờ kết nối
        int client = accept(listener, NULL, NULL);
        printf("New client %d accepted in thread %ld with pid %d\n", client, 
        pthread_self(), getpid());

        // Chờ dữ liệu từ client
        int ret = recv(client, buf, sizeof(buf), 0);
        buf[ret] = 0;
        puts(buf);

        // Trả lại kết quả cho client
        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
        send(client, msg, strlen(msg), 0);

        // Đóng kết nối
        close(client);
    }
    return NULL;
}

