#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

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

    struct pollfd fds[64];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    int num_clients = 0;
    int users[64];      // Mang socket client da dang nhap
    char *user_ids[64]; // Mang id client da dang nhap
    int num_users = 0;  // So luong client da dang nhap

    while (1)
    {
        // Chờ đến khi sự kiện xảy ra
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        if (ret == 0)
        {
            printf("Timed out!!!\n");
            continue;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & (POLLIN || POLLERR))
            {
                if(i == 0)
                {// Yeu cau ket noi moi
                    int client = accept(listener, NULL, NULL);
                    char *msg = "Vui long nhap username va password.\n";
                    send(client, msg, strlen(msg), 0);
                    if(nfds < 64)
                    {   
                        printf("New client connected: %d\n", client);
                        fflush(stdout);
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    }
                    else
                    {
                        // Dang co qua nhieu ket noi
                        char *msg = "He thong qua tai, vui long thu lai sau,\n";
                        send(client, msg, strlen(msg), 0);
                        close(client);
                    }
                }
                else
                {
                    int ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        close(i);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", i, buf);
                        fflush(stdout);
                        int client = fds[i].fd;

                        // Kiem tra trang thai dang nhap
                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client)
                                break;
                            
                        if (j == num_users) // Chua dang nhap
                        {
                            // Xu ly cu phap lenh dang nhap
                            char un[32], pw[32];
                            ret = sscanf(buf, "%s %s", un, pw);
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
                            printf("Failed ot run command\n");

                            while(fgets(output, sizeof(output), fp) != NULL) {
                                fwrite(output,1, strlen(output), f);
                                write(client, output, strlen(output));
                            }
                            fclose(f);
                            pclose(fp);
                        }
                    }
                }
            }
        }
    }
    close(listener);    
    return 0;
}