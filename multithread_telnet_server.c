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

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
void* thread_proc(void *arg);
int num_clients = 0;
int users[64];      // Mang socket client da dang nhap
char *user_ids[64]; // Mang id client da dang nhap
int num_users = 0;  // So luong client da dang nhap

void process_request(int client, char *buf)
{
    // Kiem tra trang thai dang nhap
    int j = 0;
    for (; j < num_users; j++)
        if (users[j] == client)
            break;
        
    if (j == num_users) // Chua dang nhap
    {
        // Xu ly cu phap lenh dang nhap
        char un[32], pw[32];
        int ret = sscanf(buf, "%s %s", un, pw);
        if (ret == 2)
        {   
            FILE *fp;
            fp = fopen("user + pass.txt", "r");
            while(1)
            {
                char username[32];
                char password[32]; 
                fscanf(fp, "%s %s", username, password);
                if (feof(fp))
                {
                    char *msg = " Ten dang nhap hoac mat khau khong dung. Vui long thu lai.\n";
                    send(client, msg, strlen(msg), 0);
                    break;
                }
                if (strcmp(un, username) == 0 && strcmp(pw, password) == 0)
                {
                    char *msg = "Dang nhap thanh cong.\n";
                    send(client, msg, strlen(msg), 0);

                    // Luu vao mang user
                    users[num_users] = client;
                    user_ids[num_users] = malloc(strlen(un) + 1);
                    strcpy(user_ids[num_users], un);
                    num_users++;
                    break;
                }
            }
            fclose(fp);
        }
        else
        {
            char *msg = "Sai tham so. Hay nhap lai.\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else // Da dang nhap
    {
        char output[1035];
        FILE *fp;
        FILE *f;
        f = fopen("out.txt", "w");
        fp = popen(buf, "r");
        if(fp == NULL)
        printf("Failed to run command\n");

        while(fgets(output, sizeof(output), fp) != NULL) {
            fwrite(output, 1, strlen(output), f);
            write(client, output, strlen(output));
        }
        fclose(f);
        pclose(fp);
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
        pthread_mutex_lock(&counter_mutex);
        process_request(client, buf);
        pthread_mutex_unlock(&counter_mutex);
    }
    close(client);
}

