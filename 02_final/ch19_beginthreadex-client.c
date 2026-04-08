// gcc ch19_beginthreadex-client.c -o ch19_beginthreadex-client.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 120

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    connect(client_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    printf("Client> connection established...\n");

    // 계산할 피연산자 및 연산자를 입력받고 서버로 전송한 뒤 결과 수신
    int  flag = 1;
    int  operand_count;
    char buf[BUF_LEN];
    
    while (flag) {
        // 사용자로부터 계산 자료 입력... (피연산자 개수, 피연산자, 연산자)
        printf("Client: Enter the number of operands: ");
        scanf("%d", &operand_count);
        buf[0] = (char)operand_count;

        for (int i = 0; i < operand_count; i++) {
            printf("- Enter operand %d: ", i+1);
            scanf("%d", &buf[1+i*sizeof(int)]);
        }
        rewind(stdin);
        
        printf("> Enter operator: ");
        scanf("%c", &buf[1+operand_count*sizeof(int)]);
        send(client_sock, buf, 2+operand_count*sizeof(int), 0);

        // 결과 수신 및 출력
        int bytes_len;
        int recv_total = sizeof(bytes_len); // 수신 목표 바이트 수
        int recv_sum   = 0;                 // 현재 수신된 바이트 수
        
        while (recv_sum < recv_total) {
            bytes_len = recv(client_sock, &buf[recv_sum], recv_total-recv_sum, 0);
            
            if (bytes_len <= 0) {
                printf("<ERROR> recv failed.\n");
                flag = 0;
                break;
            } else {
                recv_sum = recv_sum + bytes_len;
                printf(
                    "Client> recv %d bytes. sum=%d, total=%d\n",
                    bytes_len, recv_sum, recv_total
                );
            }
        }

        if (flag == 1) {
            bytes_len = *((int*)buf);
            printf("Client> Result received from server = %d\n", bytes_len);
        }
    }
    closesocket(client_sock);
    printf("Client> close socket...\n");
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
