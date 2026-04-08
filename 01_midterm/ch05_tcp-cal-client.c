// gcc midterm/ch05_tcp-cal-client.c -o midterm/ch05_tcp-cal-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

void error_handling(char *msg);

int main() {
    printf("Client> Run program\n");

    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (client_sock == INVALID_SOCKET) {
        printf("socket() error\n");
        WSACleanup();
        return 1;
    } else {
        printf("Client> socket() success\n");
    }

    SOCKADDR_IN server_addr;                                   // 주소표현 구조체
    memset(&server_addr, 0, sizeof(server_addr));              // 메모리 공간을 0으로 초기화
    server_addr.sin_family           = AF_INET;                // IPv4
    server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // IP 주소(Loopback): 32비트 정수형(빅 엔디안)으로 변환
    server_addr.sin_port             = htons(9000);            // Port: 호스트 순서 to 네트워크 순서 (short)

    result = connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    if (result == SOCKET_ERROR) {
        printf("connect() error, code=%d\n", WSAGetLastError());
        closesocket(client_sock);
        WSACleanup();
        return 1;
    } else {
        printf("Client> Connected to server successfully.\n");
    }

    int operand_count;
    char buf[BUF_LEN]; // char 배열 buf[]에 저장

    while (1) {
        printf("Client> Enter the number of operands: "); // 피연산자 수 입력
        scanf("%d", &operand_count); // 저장 위치
        buf[0] = operand_count;   // 저장 위치

        for (int i = 0; i < operand_count; i++) {                    // 피연산자 수만큼
            printf("Client> Enter the %dth operand: ", i + 1); // 피연산자 입력
            scanf("%d", &buf[1 + sizeof(int) * i]);         // 저장 위치
        }
        getchar(); // 이전 엔터 삭제

        printf("Client> Enter the operator(+/-/*): ");   // 연산자 입력
        scanf("%c", &buf[1 + sizeof(int) * operand_count]); // 저장 위치

        // ================================================== //
        // send(): 계산할 데이터를 서버 소켓으로 송신
        // ================================================== //
        int bytes_len = send(
            client_sock, buf,
            1 + sizeof(int) * operand_count + 1, // 버퍼 크기: 피연산자 수(1) + 피연산자 + 연산자(1)
            0 // 플래그
        );
        if (bytes_len == SOCKET_ERROR) {
            printf("send() error, code = %d\n", WSAGetLastError());
            break;
        } else { // send() OK
            printf("Client> sent to server %d bytes.\n", bytes_len);
        }

        // ================================================== //
        // recv(): 계산한 데이터를 서버 소켓으로부터 수신
        // ================================================== //
        int recv_sum    = 0; // 수신 누적치: 현재까지 수신량을 누적
        int recv_total = 4; // 수신 목표치: result 정수 4B 수신 예정

        while (recv_sum < recv_total) {
            bytes_len = recv(
                client_sock, &buf[recv_sum],
                sizeof(buf) - recv_sum, // 버퍼 크기
                0 // 플래그
            );
            if (bytes_len == 0) { // 상대가 socket을 close한 상태
                printf("Client> The server closed the socket.\n");
                closesocket(client_sock);
                WSACleanup();
                return 1;
            } else if (bytes_len == SOCKET_ERROR) {
                printf("recv() error\n");
                closesocket(client_sock);
                WSACleanup();
                return 1;
            } else {
                recv_sum += bytes_len;
            }
        }

        // 결과 수신 완료
        result = *((int*)buf);
        printf("Client> Calculation result received from the server: %d\n", result);
    }
    closesocket(client_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    printf("[ERROR] %s\n", msg);
    exit(1);
}
