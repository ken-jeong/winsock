// gcc midterm/ch09_sockopt.c -o midterm/ch09_sockopt.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
        
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    int a;
    int b = sizeof(a);

    // 1. 송신 버퍼 크기 확인
    result = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&a, &b);
    printf("1> Send buffer size: %d bytes\n", a);

    // 2. 송신 버퍼 크기를 2배로 설정
    a = a * 2; // 2배로 설정
    result = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&a, sizeof(a));

    // 3. 변경 후 송신 버퍼 크기 재확인
    b = sizeof(a);
    result = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&a, &b);
    printf("2> Send buffer size: %d bytes\n", a);

    closesocket(sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
