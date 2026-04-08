// gcc ch23_OIO-CompRout-server.c -o ch23_OIO-CompRout-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1024

void CALLBACK read_completion_routine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK write_completion_routine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void error_handling(char *msg);

typedef struct {
    SOCKET client_sock;
    char   buf[BUF_LEN];
    WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = WSASocket(        // < WSASocket() >
        PF_INET, SOCK_STREAM, IPPROTO_TCP, // 주소, 소켓, 프로토콜
        NULL, 0,                           // 보통 NULL, 0
        WSA_FLAG_OVERLAPPED                // 플래그
    );

    // 넌블로킹(Non-blocking) 모드로 소켓 설정 및 accept() 준비
    int mode   = 1;
    int result = ioctlsocket(server_sock, FIONBIO, &mode);
    if (result == SOCKET_ERROR) {
        error_handling("ioctlsocket() error");
    }

    // 3. 서버 주소 설정 및 bind, listen
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);
    
    bind(server_sock, (SOCKADDR*) &server_addr, sizeof(server_addr));
    
    listen(server_sock, 5);
    
    while (1) {
        // 4. 콜백(Callback) 함수가 호출될 수 있는 Alertable Wait 상태로 진입
        SleepEx(100, TRUE);

        // 5. 비동기(넌블로킹) accept() 수행
        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);
        SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
        
        if (client_sock == INVALID_SOCKET) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                continue;
            } else {
                error_handling("accept() error");
            }
        }
        printf("Client connected...\n");

        // 6. 비동기 수신(WSARecv) 함수 호출
        LPWSAOVERLAPPED lp_overlapped = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
        memset(lp_overlapped, 0, sizeof(WSAOVERLAPPED));

        LPPER_IO_DATA io_data = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
        io_data->client_sock = client_sock;
        (io_data->wsaBuf).buf = io_data->buf;
        (io_data->wsaBuf).len = BUF_LEN;
        lp_overlapped->hEvent = (HANDLE)io_data; // 구조체 주소를 hEvent 멤버에 저장하여 콜백 함수로 전달

        DWORD recvd_bytes, flags = 0;

        WSARecv(
            client_sock, &(io_data->wsaBuf), 1,    // 소켓, 버퍼, 버퍼 수
            &recvd_bytes, &flags,                  // 바이트 수, 플래그
            lp_overlapped, read_completion_routine // overlapped, completion_routine
        );
    }
    
    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

// 수신 완료 콜백 함수
void CALLBACK read_completion_routine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED overlapped, DWORD flags) {
    // 매개변수로 전달된 overlapped 내 hEvent 필드를 통해 사용자 데이터에 접근
    LPPER_IO_DATA io_data    = (LPPER_IO_DATA)(overlapped->hEvent);
    SOCKET        client_sock = io_data->client_sock;
    LPWSABUF      bufInfo    = &(io_data->wsaBuf);

    // 1. 클라이언트가 연결을 종료한 경우 (수신된 바이트가 0인 경우) 처리
    if (szRecvBytes == 0) {
        closesocket(client_sock);
        free(overlapped->hEvent); // io_data 메모리 해제
        free(overlapped);         // overlapped 메모리 해제
        printf("Client disconnected...\n");
    } else { // 에코(Echo) 송신
        // 2. 데이터 수신이 완료된 경우, 수신된 데이터를 클라이언트에게 다시 전송
        bufInfo->len = szRecvBytes; // 실제 수신한 바이트 수만큼 설정
        
        DWORD sent_bytes;

        WSASend(
            client_sock, bufInfo, 1,             // 소켓, 버퍼, 버퍼 수
            &sent_bytes, 0,                      // 바이트 수, 플래그
            overlapped, write_completion_routine // overlapped, completion_routine
        );
    }
}

// 송신 완료 콜백 함수
void CALLBACK write_completion_routine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED overlapped, DWORD flags) {
    // 1. 클라이언트로의 데이터 전송이 완료된 경우, 새로운 데이터 수신 대기
    LPPER_IO_DATA io_data    = (LPPER_IO_DATA)(overlapped->hEvent);
    SOCKET        client_sock = io_data->client_sock;
    LPWSABUF      bufInfo    = &(io_data->wsaBuf);

    bufInfo->len = BUF_LEN; // 버퍼 길이를 원래 최대 크기로 복구

    DWORD recvd_bytes, flags = 0;

    WSARecv(
        client_sock, bufInfo, 1,            // 소켓, 버퍼, 버퍼 수
        &recvd_bytes, &flags,               // 바이트 수, 플래그
        overlapped, read_completion_routine // overlapped, completion_routine
    );
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
