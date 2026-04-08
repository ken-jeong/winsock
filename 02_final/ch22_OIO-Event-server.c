// gcc ch22_OIO-Event-server.c -o ch22_OIO-Event-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1024

void error_handling(char *msg);

int main(void) {
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
    // 이벤트 등록
    overlapped.hEvent = event_obj;

    // 버퍼 구조체 설정
    char buf[BUF_LEN];
    WSABUF wsabuf;
    wsabuf.len = BUF_LEN;
    wsabuf.buf = buf;

    printf("Server> Calling WSARecv().\n");

    DWORD bytes_len = 0, flags = 0;

    int result = WSARecv(        // < WSARecv() >
        client_sock, &wsabuf, 1, // 소켓, 버퍼, 버퍼 수
        &bytes_len, &flags,      // 바이트 수, 플래그
        &overlapped, NULL        // overlapped, completion_routine
    );
    
    if (result == SOCKET_ERROR) {
        if (WSAGetLastError() == WSA_IO_PENDING) {
            printf("Server> Background data receiving...\n");
            
            WSAWaitForMultipleEvents( // +-< WSAWaitForMultipleEvents() >-+
                1,    &event_obj,     // | number_of_events, event_array  |
                TRUE, WSA_INFINITE,   // | all_waiting,      timeout      |
                FALSE                 // | winsock: FALSE                 |
            );                        // +--------------------------------+

            WSAGetOverlappedResult(       // < WSAGetOverlappedResult() >
                client_sock, &overlapped, // socket, &overlapped
                &bytes_len,               // 바이트 수
                FALSE,                    // 대기
                &flags                    // 플래그
            );

            printf("Server> Received message: %s\n", buf);
        } else {
            error_handling("WSARecv() error");
        }
    } else { // 동기적으로 데이터 수신이 바로 완료되었을 때의 처리
        printf("Server> Received message immediately: %s\n", buf);
    }

    WSACloseEvent(event_obj); // < WSACloseEvent() >

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
