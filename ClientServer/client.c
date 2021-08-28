#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int check_port(char *arg)
{
    int length = strlen(arg);

    if (length > 5)
        return -1;

    for (int i = 0; i < length; i++)
        if (!isdigit(arg[i]))
            return -1;

    return atoi(arg);
}

int main(int argc, char **argv)
{
    // Check args
    if (argc != 2)
    {
        errno = EINVAL;
        perror("Wrong number of args");
        return errno;
    }

    // Parse port
    int port = check_port(argv[1]);
    if (port == -1)
    {
        errno = EINVAL;
        perror("Incorrect port");
        return errno;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return errno;
    }

    // Create IP
    struct in_addr ip;
    ip.s_addr = inet_addr("127.0.0.1");

    // Create socket address
    struct sockaddr_in address;
    address.sin_addr = ip;
    address.sin_family = AF_INET;
    address.sin_port = port;

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("connect");
        return errno;
    }

    char msg[256];
    if (recv(sock, msg, sizeof(msg), 0) == -1)
    {
        perror("recv");
        return errno;
    }
    else
    {
        printf("Client received [%s]\n", msg);

        if (strcmp(msg, "Server is busy. Wait for your turn.") == 0)
        {
            if (recv(sock, msg, sizeof(msg), 0) == -1)
            {
                perror("recv");
                return errno;
            }
            else
            {
                printf("Client received [%s]\n", msg);
            }
        }
    }

    close(sock);
    return 0;
}
