// gcc midterm/ch08_gethostbyaddr.c -o midterm/ch08_gethostbyaddr.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void error_handling(char *msg);

int main() {
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
    
    // IP 주소 입력
    char ip_addr[50];
    printf("> Enter IP address: ");
    scanf("%s", ip_addr);
    printf(">> Input IP address: %s\n", ip_addr);
    
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = inet_addr(ip_addr);

    struct hostent* host;
    host = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET); // < gethostbyaddr() >
    if (!host) {
        printf("<ERROR> gethostbyaddr() error (code = %d)\n",
            WSAGetLastError());
    }

    // 결과 출력
    printf("> Official name: %s\n", host->h_name);
    for (int i = 0; host->h_aliases[i]; i++) {
        printf("- Aliases %d: %s\n", i + 1, host->h_aliases[i]);
    }
    printf("> Address type: %s\n",
        (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
    for (int i = 0; host->h_addr_list[i]; i++) {
        printf("- IP addr %d: %s\n", i + 1,
            inet_ntoa(*((struct in_addr*)host->h_addr_list[i])));
    }

    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
