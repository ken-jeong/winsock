// gcc midterm/ch07_shutdown-client.c -o midterm/ch07_shutdown-client.exe -lws2_32

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
    fp = fopen("receive.dat", "wb");
    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    char buf[BUF_LEN];
    
    while (1) {
        int bytes_len = recv(client_sock, buf, BUF_LEN, 0);
        if (bytes_len != 0) {
            fwrite((void*)buf, 1, bytes_len, fp);
        } else {
            printf("Client> server execute shutdown.\n");
            break;
        }
    }

    puts("Received file data");
    send(client_sock, "Thank you", 10, 0);

    fclose(fp);
    closesocket(client_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
