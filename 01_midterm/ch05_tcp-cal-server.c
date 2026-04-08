// gcc midterm/ch05_tcp-cal-server.c -o midterm/ch05_tcp-cal-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

int calculate(int operand_count, int opnds[], char op);
void error_handling(char *msg);

int main(void) {
    printf("Server> Run program\n");

    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    } else {
        printf("socket() success\n");
    }

    SOCKADDR_IN server_addr;                              // 주소표현 구조체
    memset(&server_addr, 0, sizeof(server_addr));         // 메모리 공간을 0으로 초기화
    server_addr.sin_family      = AF_INET;                // IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP 주소(Loopback): 32비트 정수형(빅 엔디안)으로 변환
    server_addr.sin_port        = htons(9000);            // Port: 호스트 순서 to 네트워크 순서 (short)

    result = bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    if (result == SOCKET_ERROR) {
        printf("bind() error, code=%d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    } else {
        printf("Server> Binded to client successfully.\n");
    }

    result = listen(server_sock, 5); // 대기 큐의 크기는 5

    if (result == SOCKET_ERROR) {
        printf("listen() error, code=%d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    // client의 연결 요청을 처리해야 함 -> table 앉힘-주문 받기
    char operand_count;
    char buf[BUF_LEN];

    while (1) { // 여러 client를 loop 돌면서 반복해서 처리하기 위한 loop
        printf("Server> Waiting for client connection requests...\n");

        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);
        SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
        
        if (client_sock == INVALID_SOCKET) {
            printf("accept() error\n");
            closesocket(server_sock);
            WSACleanup();
            return 1;
        }

        // 새로운 client 연결 요청을 위한 소켓 생성 완료
        printf("Server> 새로운 client(IP:%s, Port:%d)가 연결되었습니다.\n",
            inet_ntoa(client_addr.sin_addr), // Network to ASCII
            ntohs(client_addr.sin_port)      // 네트워크 순서 to 호스트 순서 (short)
        );

        int flag = 1;
        while (flag) { // 특정 연결된 client (Accept된 Client)의 여러 요청 반복 처리
            // ================================================== //
            // recv(): 계산할 데이터를 클라이언트 소켓으로부터 수신
            // ================================================== //
            // 메시지의 고정 부분(operand_count, 1byte) 읽음 (recv -1)
            int bytes_len = recv( // 수신된 메시지의 첫 번째 바이트 읽기
                client_sock,
                &operand_count, // 버퍼
                1,        // 버퍼 크기
                0         // 플래그
            );

            if (bytes_len == 0) { // 상대가 socket을 close한 상태
                printf("Server> The client closed the socket.\n");
                break;
            } else if (bytes_len == SOCKET_ERROR) {
                printf("recv() error, code=%d\n", WSAGetLastError());
                break;
            }

            // 1 바이트 수신한 상황...피연산자의 갯수 buf[0]에 저장된 상태
            // operand_count = buf[0];

            // 1.2. 메시지의 나머지 부분을 마저 읽음 (recv -2)
            int recv_sum = 0; // 읽은 누적치
            int recv_total = (int)operand_count * sizeof(int) + 1; // 읽어야될 메시지의 크기가 정해짐

            // recv loop 원하는 크기가 읽혀질 때까지 반복 recv 수행
            while (recv_sum < recv_total) {
                bytes_len = recv(
                    client_sock,
                    &buf[recv_sum],   // 버퍼
                    recv_total - recv_sum, // 버퍼 크기
                    0                   // 플래그
                );

                if (bytes_len == 0) { // 상대가 socket close 한 상태
                    printf("Server> The client closed the socket.\n");
                    flag = 0;
                    break;
                } else if (bytes_len == SOCKET_ERROR) {
                    printf("recv() error, code=%d\n", WSAGetLastError());
                    flag = 0;
                    break;
                }
                recv_sum += bytes_len;
            }

            // 계산하기
            int result = calculate(operand_count, buf, buf[recv_total - 1]);

            // ================================================== //
            // send(): 계산한 데이터를 클라이언트 소켓으로 송신
            // ================================================== //
            bytes_len = send(
                client_sock,
                (char*)&result, // 버퍼
                sizeof(result), // 버퍼 크기
                0               // 플래그
            );

            if (bytes_len == SOCKET_ERROR) {
                printf("send() error, code = %d\n", WSAGetLastError());
                break;
            } else { // send OK
                printf("Server> sent result to client %d bytes.\n", bytes_len);
            }
        }
        closesocket(client_sock);
    }
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

int calculate(int operand_count, int opnds[], char op) {
    int result = opnds[0];

    switch (op) {
    case '+': // p1, p2, p3 -> result = p1 + p2 + p3;
        for (int i = 1; i < operand_count; i++) {
            result += opnds[i];
        }
        break;

    case '-': // p1, p2, p3 -> result = p1 - p2 - p3;
        for (int i = 1; i < operand_count; i++) {
            result -= opnds[i];
        }
        break;

    case '*': // p1, p2, p3 -> result = p1 * p2 * p3;
        for (int i = 1; i < operand_count; i++) {
            result *= opnds[i];
        }
        break;
    }

    return result;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
