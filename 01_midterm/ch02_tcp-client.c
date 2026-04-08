// gcc midterm/ch02_tcp-client.c -o midterm/ch02_tcp-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (client_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);
    
    // 서버에 연결 요청
    result = connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    if (result == SOCKET_ERROR) {
        error_handling("connect() error!");
    }
    printf("> Client requested a TCP connection.\n");

    // 반복해서 send() 함수를 호출하여 서버로 데이터를 전송
    int  index;
    char buf[BUF_LEN];

    // 전송할 데이터 준비
    for (index = 0; index < BUF_LEN; index++) {
        buf[index] = index;
    }

    for (index = 0; index < 5; index++ ) {
        int bytes_len = send(client_sock, buf, BUF_LEN, 0);

        if (bytes_len == SOCKET_ERROR) {
            printf("<error> Error occurred during send(). code(%d)\n", WSAGetLastError());
        } else if (bytes_len > 0) {
            printf("%d> Sent %d bytes via send().\n", index+1, bytes_len);
        }
        
        Sleep(1000);
    }
    result = closesocket(client_sock);
    result = WSACleanup();
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
