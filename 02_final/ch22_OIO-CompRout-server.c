// gcc ch22_OIO-CompRout-server.c -o ch22_OIO-CompRout-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1024

void CALLBACK completion_routine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void error_handling(char *msg);

char  buf[BUF_LEN];
DWORD bytes_len = 0;

int main() {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = WSASocket(        // < WSASocket() >
        PF_INET, SOCK_STREAM, IPPROTO_TCP, // 주소, 소켓, 프로토콜
        NULL, 0,                           // 보통 NULL, 0
        WSA_FLAG_OVERLAPPED                // 플래그
    );

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    listen(server_sock, 5);

    SOCKADDR_IN client_addr;
    int         sizeof_addr = sizeof(client_addr);
    SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);

    printf("Server> accept() completed.\n");

    // 이벤트 객체 생성
    WSAEVENT event_obj = WSACreateEvent();

    // Overlapped 구조체 초기화
    WSAOVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    // 버퍼 구조체 설정
    WSABUF wsabuf;
    wsabuf.len = BUF_LEN;
    wsabuf.buf = buf;

    printf("Server> recv().\n");
    
    DWORD flags = 0;
    int result = WSARecv(               // < WSARecv() >
        client_sock, &wsabuf, 1,        // 소켓, 버퍼, 버퍼 수
        &bytes_len, &flags,             // 바이트 수, 플래그
        &overlapped, completion_routine // overlapped, completion_routine
    );

    if (result == SOCKET_ERROR) {
        if (WSAGetLastError() == WSA_IO_PENDING) {
            printf("Server> Background data receive...\n");
        }
    }

    // 이벤트 발생 대기하기 (Alertable Wait 상태 진입)
    printf("Server> enter alertable wait.\n");

    DWORD idx = WSAWaitForMultipleEvents( // +-< WSAWaitForMultipleEvents() >-+
        1,     &event_obj,                // | number_of_events, event_array  |
        FALSE, WSA_INFINITE,              // | only_one_waiting, timeout      |
        TRUE                              // | APC                            |
    );                                    // +--------------------------------+

    if (idx == WAIT_IO_COMPLETION) {
        printf("Server> Overlapped I/O Completed...\n");
    } else {
        error_handling("WSARecv() error");
    }

    WSACloseEvent(event_obj); // < WSACloseEvent() >

    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void CALLBACK completion_routine(DWORD dw_error, DWORD sz_numof_bytes, LPWSAOVERLAPPED lp_overlapped, DWORD flags) {
    if (dw_error != 0) {
        error_handling("Comp> completion_routine error");
    } else {
        bytes_len = sz_numof_bytes;
        printf("Comp> Received message: %s \n", buf);
    }
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
