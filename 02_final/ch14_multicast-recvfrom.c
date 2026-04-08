// gcc ch14_multicast-recvfrom.c -o ch14_multicast-recvfrom.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for struct ip_mreq
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 30

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET recv_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN recv_addr;
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family      = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port        = htons(9000);

    bind(recv_sock, (SOCKADDR*)&recv_addr, sizeof(recv_addr));

    // 멀티캐스트 주소 수신하도록 설정하기
    struct ip_mreq opt;
    opt.im_interface.s_addr = htonl(INADDR_ANY);
    opt.im_multiaddr.s_addr = inet_addr("224.1.1.2"); // Class D Multicast Address

    setsockopt( // < setsockopt() >
        recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        (void*)&opt, sizeof(opt)
    );

    // 반복적으로 수신하기 -> 수신 여부 확인
    while (1) {
        char buf[BUF_LEN];
        
        int bytes_len = recvfrom(
            recv_sock, buf, BUF_LEN-1, 0,
            NULL, 0
        );

        if (bytes_len < 0) { // bytes_len > 0, bytes_len < 0: 오류 상황
            break;
        }

        buf[bytes_len] = 0; // 스트링의 끝 표시
        printf("%s", buf);
    }

    closesocket(recv_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
