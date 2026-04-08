// gcc ch21_asyncNoti-WSAEvent-server.c -o ch21_asyncNoti-WSAEvent-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100

void compress_sockets(SOCKET socket_array[], int deleted_index, int total_count);
void compress_events(WSAEVENT event_array[], int deleted_index, int total_count);
void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(9000);
    
    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    listen(server_sock, 5);
    
    WSAEVENT event_obj = WSACreateEvent(); // < WSACreateEvent() >
    if (event_obj == WSA_INVALID_EVENT) {
        error_handling("WSACreateEvent() error");
    }

    // ================================================== //
    // 2. WSAEventSelect()
    // 소켓에 이벤트 객체를 연결하고, 감지할 네트워크 이벤트를 등록한다.
    // ================================================== //
    int result = WSAEventSelect(
        server_sock, event_obj, FD_ACCEPT
    );

    if (result == SOCKET_ERROR) {
        error_handling("WSAEventSelect() error");
    }

    // 소켓과 이벤트 객체를 배열에 저장
    SOCKET   socket_array[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT event_array[WSA_MAXIMUM_WAIT_EVENTS];
    int socket_count = 0;

    socket_array[socket_count]  = server_sock;
    event_array[socket_count++] = event_obj;
    char buf[BUF_LEN];
    
    while (1) {
        result = WSAWaitForMultipleEvents( // +-< WSAWaitForMultipleEvents() >-+
            socket_count, event_array,     // | number_of_events, event_array  |
            FALSE,        WSA_INFINITE,    // | only_one_waiting, timeout      |
            FALSE                          // | winsock: FALSE                 |
        );                                 // +--------------------------------+

        int first_signaled_index = result - WSA_WAIT_EVENT_0; // Signalled 상태 이벤트 객체 확인

        for (int i = first_signaled_index; i < socket_count; i++) {
            result = WSAWaitForMultipleEvents( // +-< WSAWaitForMultipleEvents() >-+
                1,    &event_array[i],         // | number_of_events, event_array  |
                TRUE, 0,                       // | all_waiting,      timeout      |
                FALSE                          // | winsock: FALSE                 |
            );                                 // +--------------------------------+

            if ((result == WSA_WAIT_FAILED || result == WSA_WAIT_TIMEOUT)) {
                continue;
            } else {
                WSANETWORKEVENTS networkEvents; // 발생한 이벤트 구체화 때 사용하는 구조체 변수
                
                WSAEnumNetworkEvents( // +-< WSAEnumNetworkEvents() >-+
                    socket_array[i],  // | socket                     |
                    event_array[i],   // | event                      |
                    &networkEvents    // | result                     |
                );                    // +----------------------------+

                /* 4.1. FD_ACCEPT 이벤트 처리 */
                if (networkEvents.lNetworkEvents & FD_ACCEPT) {
                    if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                        puts("Accept Error");
                        break;
                    }
                    
                    SOCKADDR_IN client_addr;
                    int         sizeof_addr = sizeof(client_addr);
                    SOCKET      client_sock = accept(socket_array[i], (SOCKADDR*)&client_addr, &sizeof_addr);

                    WSAEVENT new_event_obj = WSACreateEvent(); // 새로운 이벤트 생성
                    WSAEventSelect( // 이벤트와 소켓 연결 및 등록
                        client_sock, new_event_obj, FD_READ | FD_CLOSE
                    );

                    socket_array[socket_count] = client_sock;
                    event_array[socket_count++] = new_event_obj;
                    puts("connected new client...");
                }

                /* 4.2. FD_READ 이벤트 처리 */
                if (networkEvents.lNetworkEvents & FD_READ) {
                    if (networkEvents.iErrorCode[FD_READ_BIT] != 0) {
                        puts("Read Error");
                        break;
                    }

                    int bytes_len = recv(
                        socket_array[i], buf, sizeof(buf), 0
                    );

                    buf[bytes_len] = 0;
                    printf("%s\n", buf);
                    send(
                        socket_array[i], buf, bytes_len, 0
                    );
                }

                /* 4.3. FD_CLOSE 이벤트 처리 */
                if (networkEvents.lNetworkEvents & FD_CLOSE) {
                    if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
                        puts("Close Error");
                        break;
                    }

                    WSACloseEvent(event_array[i]); // < WSACloseEvent() >

                    closesocket(socket_array[i]);

                    socket_count--;
                    compress_sockets(socket_array, i, socket_count);
                    compress_events(event_array, i, socket_count);
                    i--;
                }
            }
        }
    }
    WSACleanup();
    return 0;
}

// 소켓 정보 배열 정리
void compress_sockets(SOCKET socket_array[], int deleted_index, int total_count) {
    for (int i = deleted_index; i < total_count; i++) {
        socket_array[i] = socket_array[i+1];
    }
}

// 이벤트 정보 배열 정리
void compress_events(WSAEVENT event_array[], int deleted_index, int total_count) {
    for (int i = deleted_index; i < total_count; i++) {
        event_array[i] = event_array[i+1];
    }
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
