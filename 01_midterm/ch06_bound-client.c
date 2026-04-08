// gcc midterm/ch06_bound-client.c -o midterm/ch06_bound-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 30

void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    char buf[BUF_LEN];
    
    // connect(sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    while (1) {
        printf("Insert message(q to quit): ");
        scanf("%s", buf);
        if (!strcmp(buf,"q") || !strcmp(buf,"Q")) {
            break;
        }

        // send(client_sock, buf, strlen(buf), 0);
        sendto(
            client_sock, buf, strlen(buf), 0,
            (SOCKADDR*)&server_addr, sizeof(server_addr)
        );
        printf(
            "1. sent msg(%s) to(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port)
        );

        // send(client_sock, buf, strlen(buf), 0);
        sendto(
            client_sock, buf, strlen(buf), 0,
            (SOCKADDR*)&server_addr, sizeof(server_addr)
        );
        printf(
            "2. sent msg(%s) to(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port)
        );

        // send(client_sock, buf, strlen(buf), 0);
        sendto(
            client_sock, buf, strlen(buf), 0,
            (SOCKADDR*)&server_addr, sizeof(server_addr)
        );
        printf(
            "3. sent msg(%s) to(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port)
        );
    }

    closesocket(client_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
