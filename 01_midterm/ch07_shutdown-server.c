// gcc midterm/ch07_shutdown-server.c -o midterm/ch07_shutdown-server.exe -lws2_32

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

    FILE *fp;
    fp = fopen("file_server_win.c", "rb");
    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);

    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    listen(server_sock, 5);

    SOCKADDR_IN client_addr
    int         sizeof_addr = sizeof(client_addr);
    SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
    
    char buf[BUF_LEN];
    while (1) {
        int bytes_len = fread((void*)buf, 1, BUF_LEN, fp);
        if (bytes_len < BUF_LEN) {
            send(client_sock, (char*)&buf, bytes_len, 0);
            break;
        }
        send(client_sock, (char*)&buf, BUF_LEN, 0);
    }

    shutdown(client_sock, SD_SEND);
    recv(client_sock, (char*)buf, BUF_LEN, 0);
    printf("Message from client: %s \n", buf);

    fclose(fp);
    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
