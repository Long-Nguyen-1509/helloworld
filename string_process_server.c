#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ctype.h>

char * process(char buf[])
{
    char str[256];

    int i = 0, j = 0;
    while (buf[i] != '\0')
    {
        // Tim ky tu khac dau cach dau tien tu vi tri hien tai
        while (buf[i] == ' ' && buf[i] != '\0') 
            i++;
        if (buf[i] == '\0')
            break;

        // Copy 1 tu sang xau dich
        int first = 1;
        while (buf[i] != ' ' && buf[i] != '\0')
        {
            if (first)
            {
                str[j++] = toupper(buf[i++]);
                first = 0;
            }
            else
            {
                str[j++] = tolower(buf[i++]);
            }
        }
            
        if (buf[i] == '\0')
            break;

        // Copy 1 dau cach sau tu vua copy
        if (buf[i] == ' ')
            str[j++] = buf[i++];
    }

    // Kiem tra neu tu chua dau cach cuoi thi xoa
    if (str[j - 1] == ' ')
        str[j - 1] = '\0';
    else
        str[j] = '\0';

    return str;
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
            if (fds[i].revents & (POLLIN | POLLERR))
            {
                if (i == 0)
                {
                    int client = accept(listener, NULL, NULL);
                    if (nfds < 64)
                    {
                        printf("New client connected: %d", client);
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;
                        sprintf(buf, "Xin chao. Hien co %d clients dang ket noi.", nfds);
                        send(client, buf, strlen(buf), 0);

                    }
                    else
                    {
                        // Dang co qua nhieu ket noi
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
                        printf("Received from %d: %s", i, buf);
                        
                    }
                    int client = fds[i].fd;
                    char *msg = process(buf);
                    send(client, msg, strlen(msg), 0);
                }
            }
        }
    }

    close(listener);    

    return 0;
}