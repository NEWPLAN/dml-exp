#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ATimer.h"

void error_handling(char *message);
#define KB 1000

char message[1000 * KB * 100] = "Hello World!";

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    //char message[30];
    int str_len = 0;
    int idx = 0, read_len = 0;

    int recv_size = 1000 * KB;

    if (argc < 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    recv_size = atoi(argv[3]);
    printf("Receiving size: %d B\n", recv_size);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    while (recv_size > 0)
    {
        int data_read_s = read(sock, &message[read_len], recv_size);
        if (data_read_s == -1)
        {
            error_handling("read() error!");
        }
        recv_size -= data_read_s;
        read_len += data_read_s;
    }
    if (send(sock, "1", 1, 0) != 1)
    {
        error_handling("read() error!");
        exit(0);
    }
    /*
    while (read_len = read(sock, &message[idx++], 1))
    {
        if (read_len == -1)
            error_handling("read() error!");

        str_len += read_len;
    }
*/
    //printf("Message from server: %s \n", message);
    printf("Receiving size: %d B\n", read_len);
    printf("Function read call count: %d \n", str_len);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}