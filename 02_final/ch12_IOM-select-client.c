// gcc ch12_IOM-select-client.c -o ch12_IOM-select-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 512

int recv_n(SOCKET sock, char *buf, int len, int flags);
void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    char buf[BUF_LEN + 1];
    int  result;

    while (1) {
        ZeroMemory(buf, sizeof(buf));

        // 사용자로부터 보낼 데이터 입력
        printf("\n[Data to Send] ");
        if (fgets(buf, BUF_LEN + 1, stdin) == NULL) {
            break;
        }

        result = send(client_sock, buf, strlen(buf), 0);

        if (result == SOCKET_ERROR) {
            printf("send() error\n");
            break;
        }
        
        printf("[TCP Client] Sent %d bytes.\n", result);

        // 서버로부터 데이터 수신
        result = recv_n(client_sock, buf, result, 0);

        if (result == SOCKET_ERROR) {
            printf("recv_n() error\n");
            break;
        } else if (result == 0) {
            break;
        }

        printf("[TCP Client] Received %d bytes.\n", result);

        // 수신한 데이터 출력
        buf[result]= '\0';
        printf("[Received Data] %s", buf);
    }
    
    closesocket(client_sock);
    WSACleanup();
    return 0;
}

// 지정한 크기(len)만큼 데이터를 읽어들이는 사용자 정의 수신 함수
int recv_n(SOCKET sock, char *buf, int len, int flags) {
    int  left = len;
    char *ptr = buf;

    while (left > 0) {
        int bytes_len = recv(sock, ptr, left, flags);

        if (bytes_len == SOCKET_ERROR) {
            return SOCKET_ERROR;
        } else if (bytes_len == 0) {
            break;
        }

        left -= bytes_len;
        ptr  += bytes_len;
    }
    return (len-left);
}

void error_handling(char *msg) {
    printf("<ERROR> %s\n", msg);
    exit(-1);
}
