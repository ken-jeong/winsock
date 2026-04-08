// gcc midterm/ch01_udp-server.c -o midterm/ch01_udp-server.exe -lws2_32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100

void error_handling(char *msg);

int main(void) {
    // Winsock 라이브러리 초기화
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0) {
        error_handling("WSAStartup() error");
    } else {
        printf("WSAStartup() success\n");
    }

    // 서버 소켓 생성
    SOCKET server_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    // 서버 주소 설정
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);

    // 소켓에 IP 주소와 포트 번호 할당
    result = bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    if (result == SOCKET_ERROR) {
        error_handling("bind() error");
    }

    char        buf[BUF_LEN];
    SOCKADDR_IN client_addr;
    int         sizeof_addr = sizeof(client_addr);

    int bytes_len = recvfrom(
        server_sock, buf, BUF_LEN, 0,
        (SOCKADDR*)&client_addr, &sizeof_addr
    );

    buf[bytes_len] = '\0';
    printf("> message from client: %s\n", buf);

    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
