// gcc ch21_asyncNoti-WSAEvent-client.c -o ch21_asyncNoti-WSAEvent-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1024

void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    puts("Connected...");
    char buf[BUF_LEN];

    // 데이터 송수신 루프
    while (1) {
        fputs("Input message (Q to quit): ", stdout);
        fgets(buf, BUF_LEN, stdin);

        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) { // 'q' 또는 'Q' 입력 시
            break; // 루프 종료
        }

        // 서버로 메시지 전송
        send(client_sock, buf, strlen(buf), 0);

        // 서버로부터 메시지 수신
        int bytes_len = recv(client_sock, buf, BUF_LEN-1, 0);

        buf[bytes_len] = 0;
        printf("Message from server: %s", buf);
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
