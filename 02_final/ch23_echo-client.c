// gcc ch23_echo-client.c -o ch23_echo-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
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
    
    printf("Client> Connected.\n");

    char buf[BUF_LEN];
    
    while (1) { // 메시지 송수신 루프
        fputs("Input message(Q to quit): ", stdout);
        fgets(buf, BUF_LEN, stdin);

        // 'q' 또는 'Q' 입력 시 루프 탈출 및 종료
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) {
            break;
        }

        int buf_len = strlen(buf);
        // 서버로 메시지 전송
        send(client_sock, buf, buf_len, 0);

        // 서버로부터 메시지 수신 (전송한 크기만큼 데이터가 올 때까지 반복 수신)
        int bytes_len = 0;
        while (1) {
            bytes_len += recv(client_sock, &buf[bytes_len], BUF_LEN-1, 0);
            if (bytes_len >= buf_len) {
                break;
            }
        }
        buf[buf_len] = 0;
        printf("Client> Message from server: %s\n", buf);
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
