// gcc midterm/ch04_tcp-client.c -o midterm/ch04_tcp-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    // 1. 클라이언트 소켓 생성하기
    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock == INVALID_SOCKET) {
        printf("socket() error\n");
        WSACleanup();
        return 1;
    }

    // 2. 서버로 TCP 연결 시도하기
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family           = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    server_addr.sin_port             = htons(9000);

    result = connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    if (result == SOCKET_ERROR) {
        printf("Client> connect() error, code=%d\n", WSAGetLastError());
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    // 서버와 TCP 연결 성공
    printf("Client> Connected to the server successfully.\n");
    
    int flag = 1;
    char buf[BUF_LEN];

    while (flag) {
        // 1. 사용자로부터 문자열 입력 받기 (q 입력 시 종료)
        printf("Client> Enter message (q to quit): ");
        scanf("%s", buf);

        // "q" 입력 시 처리하는 부분
        if (!strcmp(buf, "q")) {
            // 사용자가 "q"를 입력하면 루프 탈출
            break;
        }

        // 2. 입력된 문자열을 서버로 전송. send()
        int bytes_len = send(client_sock, buf, strlen(buf), 0);
        if (bytes_len == SOCKET_ERROR) {
            printf("<ERROR> send() error. code = %d\n", WSAGetLastError());
            break;
        } else { // 전송 성공
            printf("Client> Message sent successfully (%d bytes).\n", bytes_len);
        }

        // 3. 서버로부터 문자열을 수신하여 출력. recv()
        // TCP recv 특성 고려:
        // - 보낸 데이터를 한 번에 다 받지 못할 수도 있음
        // ---> 데이터를 모두 수신하기 위해 recv 루프를 돌아야 함

        int recv_total = bytes_len; // 수신 목표치 (보낸 만큼 받아야 함, 에코 서버 기준)
        int recv_sum   = 0; // 수신 누적치, 0으로 초기화
        int bytes_len;

        while (recv_sum < recv_total) {
            bytes_len = recv(client_sock, &buf[recv_sum], BUF_LEN - 1, 0);
            if (bytes_len == 0) { // 서버가 연결을 종료한 경우
                printf("Client> Server closed the connection.\n");
                flag = 0;
                break;
            }
            else { // 수신 성공
                buf[bytes_len] = 0; // 문자열의 끝(NULL) 처리
                printf("Client> Received data from server (%d bytes): %s\n",
                    bytes_len, buf);
            }
            recv_sum += bytes_len;
            buf[recv_sum] = 0; // 누적된 문자열의 끝 표시
            printf("Client> Message from server: %s (recv_sum:%d, recv_total:%d)\n",
                buf, recv_sum, recv_total
            );
        }
    }
    
    closesocket(client_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    printf("[ERROR] %s \n", msg);
    exit(1);
}
