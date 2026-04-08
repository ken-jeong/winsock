// gcc midterm/ch08_gethostbyname.c -o midterm/ch08_gethostbyname.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
    
    // 도메인 이름 입력
    char dns[50];
    printf("> Domain name: ");
    scanf("%s", dns);
    printf(">> Entered domain name: %s\n", dns);
    
    // 도메인 이름 -> IP주소 DNS 조회 요청
    struct hostent *host;
    host = gethostbyname(dns); // < gethostbyname() >
    if (!host) {
        // gethostbyname() 실패
        error_handling("gethostbyname() fail");
    }

    // 결과 출력
    printf("- Official domain name: %s\n", host->h_name); // 공식 도메인 이름

    // 별칭 목록
    for (int i = 0; host->h_aliases[i] != NULL; i++) {
        printf("> Aliases %d: %s\n", i+1, host->h_aliases[i]);
    }

    // 주소 타입: v4, v6
    printf("- Address type : %s\n",
        (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6"
    );

    // IP 주소들 출력
    for (int i = 0; host->h_addr_list[i]; i++) {
        printf("> IP addr %d: %s\n", i+1,
              inet_ntoa(*((struct in_addr *)host->h_addr_list[i]))
            );
    }

    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
