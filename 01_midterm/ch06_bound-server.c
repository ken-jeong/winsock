// gcc midterm/ch06_bound-server.c -o midterm/ch06_bound-server.exe -lws2_32

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

    SOCKET server_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }
    
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);

    if (bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error_handling("bind() error");
    }
    
    while (1) {
        char        buf[BUF_LEN];
        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);

        int bytes_len = recvfrom(
            server_sock, buf, BUF_LEN, 0,
            (SOCKADDR*)&client_addr, &sizeof_addr
        );

        buf[bytes_len] = '\0';
        printf(
            "1. received msg(%s) from(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
        );

        printf("Server> Sleep 3 sec\n");
        Sleep(3000);

        bytes_len = recvfrom(
            server_sock, buf, BUF_LEN, 0,
            (SOCKADDR*)&client_addr, &sizeof_addr
        );
        buf[bytes_len] = '\0';
        printf(
            "2. received msg(%s) from(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
        );

        printf("Server> Sleep 3 sec\n");
        Sleep(3000);

        bytes_len = recvfrom(
            server_sock, buf, BUF_LEN, 0,
            (SOCKADDR*)&client_addr, &sizeof_addr
        );
        buf[bytes_len] = '\0';
        printf(
            "3. received msg(%s) from(IP:%s,PORT:%d)\n",
            buf, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
        );
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
