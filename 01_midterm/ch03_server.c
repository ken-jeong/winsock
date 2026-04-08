// gcc midterm/ch03_server.c -o midterm/ch03_server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

void error_handling(char *msg);

int main(void) {
    printf("> Server program started.\n");

    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    // 2. 서버 소켓 생성하기...
    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    // 3. bind() 호출하여 소켓에 주소(IP, Port) 할당하기... IPv4
    struct sockaddr_in6 addr;
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET; // IPv4 주소 체계
    server_addr.sin_addr.s_addr = inet_addr("1.2.3.4");
    server_addr.sin_port        = htons(9000); // 호스트 바이트 순서 -> 네트워크 바이트 순서 (short)

    // 4. 소켓에 주소(IP, Port) 바인딩
    result = bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    if (result == SOCKET_ERROR) {
        printf("<ERROR> bind() error. code=%d\n", WSAGetLastError());
        // 서버 소켓 리소스 반환...
        closesocket(server_sock);
        // 윈속 라이브러리 해제...
        WSACleanup();
        return 1;
    }

    // 클라이언트 연결 대기 준비: DB 연결 설정 etc...
    // 5. listen: TCP 소켓을 LISTEN(대기) 상태로 변경하기
    result = listen(server_sock, 5);
    if (result == SOCKET_ERROR) {
        printf("<ERROR> listen() error. code=%d\n", WSAGetLastError());
        // 서버 소켓 리소스 반환...
        closesocket(server_sock);
        // 윈속 라이브러리 해제...
        WSACleanup();
        return 1;
    }

#if 0
    // 1. inet_ntoa() 함수 사용 예제
    char* str_ptr;
    str_ptr = inet_ntoa(server_addr.sin_addr);
    printf("IP Address string = %s\n", str_ptr);

    // 2. WSAStringToAddress() 함수 사용 예제
    //    "1.2.3.4:9000" -> IP와 Port를 모두 문자열로 표현
    char* addrStr = "5.6.7.8:9000";
    SOCKADDR_IN addr2;
    int addrSize;
    addrSize = sizeof(addr2);
    result = WSAStringToAddressA(
            addrStr, // 주소 문자열(IP, Port)
            AF_INET, // IPv4 주소 체계
            NULL,
            (SOCKADDR*)&addr2, // 변환된 주소를 저장할 구조체 포인터
            &addrSize
        );

    if (result == 0) {
        printf("IP: %#x, Port: %#x \n",
            ntohl(addr2.sin_addr.S_un.S_addr), // Big endian (네트워크 순서) 읽기
            ntohs(addr2.sin_port)    // Big endian (네트워크 순서) 읽기
            );
    }
    // 주소 구조체(addr2) -> 문자열 "5.6.7.8:9000" 로 변환하기
    char addrBuf[50];
    int bufSize;
    bufSize = sizeof(addrBuf);
    result = WSAAddressToStringA(
            (SOCKADDR*)&addr2, // 변환할 주소 구조체
            sizeof(addr2),
            NULL,
            addrBuf,
            &bufSize
        );
    if (result == 0) {
        printf("Address string = %s\n", addrBuf);
    }

    // 내 컴퓨터 CPU의 바이트 순서는 무엇일까? (Big/Little endian 확인)
    unsigned long host_addr = 0x12345678; // 호스트 바이트 순서
    unsigned long net_addr = htonl(host_addr); // 빅 엔디안(네트워크 바이트 순서)으로 변환
    printf("> Host byte order: %#x\n", host_addr);
    printf("> Network byte order (Big Endian): %#x\n", net_addr);
#endif
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
