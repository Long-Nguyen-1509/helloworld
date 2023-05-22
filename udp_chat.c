#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

int main(int argc, char *argv[]) {
    // Khai bao reciever
    int reciever = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (reciever == -1)
    {
        perror("socket() failed");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[3])); 

    // Khai bao sender
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in dest_addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    if (bind(reciever, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    struct pollfd fds[2];
    
    fds[0].fd = reciever;
    fds[0].events = POLLIN;

    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    char buf[256]; 

    while(1)
    {
        int ret = poll(fds, 2, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }
        if (ret == 0)
        {
            printf("Timed out.\n");
            continue;
        }

        if (fds[1].revents & POLLIN)
        {
            ret = recvfrom(reciever, buf, sizeof(buf), 0, NULL, NULL);
            if (ret <= 0)
            {
                break;
            }
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }

        if (fds[1].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        }
    }
}
