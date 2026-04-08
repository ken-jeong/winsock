// gcc midterm/ch02_udp-client.c -o midterm/ch02_udp-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100

// 에러 처리 함수 선언
void error_handling(char *msg);

int main(int argc, char *argv[]) {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    // UDP 소켓 생성
    SOCKET client_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (client_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }
    
    // 서버 주소 정보 설정
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    // 송신할 메시지 초기화
    char buf[BUF_LEN] = "abcdefghijklmnopqrstuvwxyz";

    // 1초 간격으로 메시지를 총 5회 전송
    for (int index = 0; index < 5; index++) {
        int buf_len = strlen(buf);

        sendto(
            client_sock, buf, buf_len, 0,
            (SOCKADDR*)&server_addr, sizeof(server_addr)
        );

        printf("%d> sent UDP message:%s\n", index+1, buf);

        Sleep(1000); // 1초 대기
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
