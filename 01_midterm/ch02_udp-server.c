// gcc midterm/ch02_udp-server.c -o midterm/ch02_udp-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100

void error_handling(char *msg);

int main(int argc, char *argv[]) {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    // UDP 소켓 생성
    SOCKET server_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    // 서버 주소 정보 설정
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);

    // 소켓에 IP 주소와 포트 번호 바인딩
    if (bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        error_handling("bind() error");
    }
    
    printf("> Waiting UDP message.\n");
    
    int index = 0;

    // 클라이언트로부터 메시지를 수신하기 위한 무한 루프
    while (1) {
        char        buf[BUF_LEN];
        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);

        // 데이터 수신 (수신 시 클라이언트의 주소 정보가 client_addr에 저장됨)
        int bytes_len = recvfrom(
            server_sock, buf, sizeof(buf) - 1, 0,
            (SOCKADDR*)&client_addr, &sizeof_addr
        );

        if (bytes_len == SOCKET_ERROR) {
            break;
        }

        buf[bytes_len] = 0; // 문자열 끝에 널 문자 삽입
        int buf_len = strlen(buf);

        // 수신한 메시지와 클라이언트 정보 출력
        printf(
            "%d> message from client(IP:%s,PORT:%d) : %s (%d bytes)\n",
            ++index,
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
            buf, buf_len
        );

        Sleep(3000); // 3초 대기
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
