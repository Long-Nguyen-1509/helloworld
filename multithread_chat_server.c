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
int num_clients = 0;
int users[64];      // Mang socket client da dang nhap
char *user_ids[64]; // Mang id client da dang nhap
int num_users = 0;  // So luong client da dang nhap

void process_request(char *buf, int client)
{
    int j = 0;
    for (; j < num_users; j++)
        if (users[j] == client)
            break;

    if (j == num_users) // Chua dang nhap
    {
        // Xu ly cu phap lenh dang nhap
        char cmd[32], id[32], tmp[32];
        int ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
        if (ret == 2)
        {
            if (strcmp(cmd, "client_id:") == 0)
            {
                char *msg = "Dung cu phap. Hay nhap tin nhan de chuyen tiep.\n";
                send(client, msg, strlen(msg), 0);

                // Luu vao mang user
                users[num_users] = client;
                user_ids[num_users] = malloc(strlen(id) + 1);
                strcpy(user_ids[num_users], id);
                num_users++;
            }
            else
            {
                char *msg = "Sai cu phap. Hay nhap lai.\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            char *msg = "Sai tham so. Hay nhap lai.\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else // Da dang nhap
    {
        char sendbuf[256];
        strcpy(sendbuf, user_ids[j]);
        strcat(sendbuf, ": ");
        strcat(sendbuf, buf);

        // Forward du lieu cho cac user
        for (int k = 0; k < num_users; k++)
            if (users[k] != client)
                send(users[k], sendbuf, strlen(sendbuf), 0);
    }
}

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
    pthread_t thread_id;
    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
            continue;
        printf("New client connected: %d\n", client);
        pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
        pthread_detach(thread_id);
    }
}

void* thread_proc(void *arg) {
    int client = *(int *)arg;
    char buf[2048];
    while (1) 
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        pthread_mutex_lock(&lock);
        process_request(buf, client);
        pthread_mutex_unlock(&lock);
    }
    close(client);
}

