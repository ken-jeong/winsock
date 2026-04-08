// gcc ch19_beginthreadex-server.c -o ch19_beginthreadex-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 120

unsigned WINAPI client_thread(LPVOID arg);
int calculate(int operand_count, int data[], char op);
void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);
    
    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    listen(server_sock, 3);

    while (1) {
        printf("Server> Waiting for client connection request...\n");

        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);
        SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
        
        printf("Server> Connected to client (IP: %s, Port: %d).\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        unsigned thread_id;

        HANDLE h_thread = _beginthreadex(       // +-< _beginthreadex() >---+
            NULL,          0,                   // | winsock: NULL, 0       |
            client_thread, (LPVOID)client_sock, // | thread_func, parameter |
            0,             &thread_id           // | initflag,    thread_id |
        );                                      // +------------------------+
        printf("Server> New thread created. Thread ID: %d\n", thread_id);
    }
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

unsigned WINAPI client_thread(LPVOID arg) {
    SOCKET client_sock = (SOCKET)arg;
    int    flag        = 1;

    while (flag) {
        printf("Thread> Waiting for calculation request...\n");

        char operand_count, buf[BUF_LEN];
        recv(client_sock, &operand_count, sizeof(operand_count), 0);
        printf("Thread> Operand count = %d\n", operand_count);

        // 현재까지 수신한 데이터 크기 (바이트 단위)
        int recv_sum   = 0;
        // 총 수신해야 할 목표 데이터 크기 (피연산자들 + 연산자 1바이트)
        int recv_total = operand_count * sizeof(int) + 1;
        
        while (recv_sum < recv_total) {
            int bytes_len = recv(client_sock, &buf[recv_sum], recv_total - recv_sum, 0);
            if (bytes_len <= 0) {
                printf("<ERROR> recv() failed or connection closed.\n");
                flag = 0;
                break;
            } else {
                recv_sum = recv_sum + bytes_len;
                printf(
                    "Thread> Received %d bytes. sum=%d, total=%d\n",
                    bytes_len, recv_sum, recv_total
                );
            }
        }

        if (flag == 1) {
            // 1. 연산 수행
            int bytes_len = calculate((int)operand_count, (int*)buf, buf[recv_total - 1]);
            printf("Thread> Calculation result = %d.\n", bytes_len);
            /** 10, 5, 2, 연산자 -이면 결과는 3 **/

            // 2. 클라이언트에게 연산 결과 전송
            send(client_sock, (char*)&bytes_len, sizeof(bytes_len), 0);
            printf("Thread> Sent the result to the client.\n");
        }
    }
    printf("Thread> Closed socket with the client.\n");
    closesocket(client_sock);
    return 0;
}

// 클라이언트가 보낸 데이터를 바탕으로 사칙연산을 수행하는 함수
int calculate(int operand_count, int data[], char op) {
    int result = data[0];

    switch (op) {
    case '+':
        for (int i = 1; i < operand_count; i++) {
            result = result + data[i];
        }
        break;
    case '-':
        for (int i = 1; i < operand_count; i++) {
            result = result - data[i];
        }
        break;
    case '*':
        for (int i = 1; i < operand_count; i++) {
            result = result * data[i];
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
