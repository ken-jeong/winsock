// gcc ch22_OIO-Event-client.c -o ch22_OIO-Event-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    
    SOCKET client_sock = WSASocket(        // < WSASocket() >
        PF_INET, SOCK_STREAM, IPPROTO_TCP, // 주소, 소켓, 프로토콜
        NULL, 0,                           // 보통 NULL, 0
        WSA_FLAG_OVERLAPPED                // 플래그
    );

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    printf("Client> Connecting...\n");

    // 이벤트 객체 생성
    WSAEVENT event_obj = WSACreateEvent();

    // 이벤트 등록
    WSAOVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = event_obj;

    // 버퍼 구조체 설정
    char buf[] = "Network is Computer!";
    WSABUF wsabuf;
    wsabuf.len = sizeof(buf);
    wsabuf.buf = buf;
    
    printf("Client> Attempting to send data after sleeping for 10 seconds...\n");
    Sleep(10000);

    printf("Client> Sending data...\n");

    DWORD bytes_len = 0;
    int result = WSASend(        // < WSASend() >
        client_sock, &wsabuf, 1, // 소켓, 버퍼, 버퍼 수
        &bytes_len, 0,           // 바이트 수, 플래그
        &overlapped, NULL        // overlapped, completion_routine
    );

    if (result == SOCKET_ERROR) {
        if (WSAGetLastError() == WSA_IO_PENDING) {
            printf("Client> Background data transmission in progress.\n");

            WSAWaitForMultipleEvents( // < WSAWaitForMultipleEvents() >
                1,    &event_obj,     // 이벤트 수, 이벤트 배열
                TRUE, WSA_INFINITE,   // 전부,     timeout
                FALSE                 // winsock: FALSE
            );

            WSAGetOverlappedResult(       // < WSAGetOverlappedResult() >
                client_sock, &overlapped, // socket, &overlapped
                &bytes_len,               // 바이트 수
                FALSE,                    // 대기
                NULL                      // 플래그
            );
        } else {
            error_handling("WSASend() error");
        }
    }
    printf("Client> Data sending complete. Size: %lu bytes\n", bytes_len);

    WSACloseEvent(event_obj); // < WSACloseEvent() >

    closesocket(client_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
