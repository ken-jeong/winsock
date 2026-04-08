// gcc midterm/ch01_tcp-server.c -o midterm/ch01_tcp-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
// #include <ws2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

void error_handling(char *msg);

int main(void) {
    // Winsock 라이브러리 초기화
    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0) {
        error_handling("WSAStartup() error");
    } else {
        printf("WSAStartup() success\n");
    }

    // 서버 소켓 생성
    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    // 서버 주소 설정
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 루프백 주소
    server_addr.sin_port        = htons(9000); // Big endian, Port 9000
    // InetPton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 소켓에 IP 주소와 포트 번호 할당
    result = bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    if (result == SOCKET_ERROR) {
        error_handling("bind() error");
    }

    // 연결 요청 대기 상태로 전환 (대기 큐 크기: 5)
    result = listen(server_sock, 5);
    
    if (result == SOCKET_ERROR) {
        error_handling("listen() error");
    }

    // 클라이언트 연결 요청 수락
    SOCKADDR_IN client_addr;
    int         sizeof_addr = sizeof(client_addr);
    SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
    
    if (client_sock == INVALID_SOCKET) {
        error_handling("accept() error");
    }

    // 데이터 수신 루프
    int  flag = 0;
    char buf[BUF_LEN];

    while (flag == 0) {
        int bytes_len = recv(client_sock, buf, BUF_LEN, 0);

        if (bytes_len > 0) { // 데이터 수신 성공
            // 수신한 문자열 출력
            buf[bytes_len] = '\0';
            printf("> recv: %s", buf);
        } else if (bytes_len == SOCKET_ERROR) { // 에러 발생
            error_handling("recv() error");
        } else if (bytes_len == 0) { // 클라이언트 연결 종료
            flag = 1; // 루프 종료
        }
    }
    
    closesocket(client_sock);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

// 에러 메시지 출력 후 프로그램 종료 함수
void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
