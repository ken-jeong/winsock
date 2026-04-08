// gcc ch14_multicast-sendto.c -o ch14_multicast-sendto.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for IP_MULTICAST_TTL option
#pragma comment(lib, "Ws2_32.lib")

#define TTL 64
#define BUF_LEN 30

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET send_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    // 전송 목적 주소지 설정
    SOCKADDR_IN multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family      = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr("224.1.1.2"); // Class D Multicast Address
    multicast_addr.sin_port        = htons(9000);

    FILE *fp;

    // 1. 파일 열기 (ch14_data.txt)
    fp = fopen("ch14_data.txt", "r");
    if (fp == NULL) {
        printf("fopen() error");
        closesocket(send_sock);
        WSACleanup();
        return 0;
    }

    // 2. 반복해서 파일 내용 send하기
    int flag = 1;
    char buf[BUF_LEN];

    if (feof(fp)) { // 빈 파일 체크
        flag = 0;   // flag에 반영, 정상: 1, 비정상: 0
    }

    while (flag == 1) { // -> 비어있으면 while 건너뛰기
        fgets(buf, BUF_LEN, fp);
        
        sendto(
            send_sock, buf, strlen(buf), 0,
            (SOCKADDR*)&multicast_addr, sizeof(multicast_addr)
        );

        // 29바이트씩 전송 ...
        printf("%s", buf);
        Sleep(2000);

        if (feof(fp)) { // 파일을 다 읽은 경우 EOF 리턴
            fseek(fp, 0, SEEK_SET); // 처음부터 다시 파일 내용 읽기 위해, 파일 포인터를 맨 앞으로 이동
            printf("\n파일을 다 읽어서 맨 앞으로 포인터 이동");
        }
    }

    fclose(fp);
    closesocket(send_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
