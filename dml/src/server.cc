#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ATimer.h"

void error_handling(char *message);

#define KB 1
#define KBS 1000
char message[1000 * KBS * 100]; //= "Hello World!";

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;

    printf("Hello\n");
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    int send_size = 1000 * KB;

    double time_dur1 = 0.f;

    if (argc < 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    send_size = atoi(argv[2]);
    printf("Sending size: %d B\n", send_size);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    {
        int opt = 1;
        // sockfd为需要端口复用的套接字
        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    //write(clnt_sock, message, sizeof(message));
    Timer t1;
    t1.start();
    if (write(clnt_sock, message, send_size) != send_size)
    {
        error_handling("accept() error");
    }
    t1.stop();
    time_dur1 = t1.milliseconds();
    //printf("Sending %d B in %lf ms\n", send_size, t1.milliseconds());
    if (read(clnt_sock, &message, 1) != 1)
    {
        error_handling("accept() error");
    }
    t1.stop();
    printf("Sending %d B in %lf ms\n", send_size, time_dur1);
    printf("Total Time: in %lf ms\n", t1.milliseconds());
    printf("Receive from client: %c\n", message[1]);

    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
