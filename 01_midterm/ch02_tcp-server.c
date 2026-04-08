// gcc midterm/ch02_tcp-server.c -o midterm/ch02_tcp-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1000

void error_handling(char *msg);

int main(void) {
    printf("> Server program started.\n");

    WSADATA wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    // 2. 서버 소켓 생성하기
    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_sock == INVALID_SOCKET) {
        error_handling("socket() error");
    }

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    // 3. bind() 함수 호출 - 서버에서 사용할 IP, port 설정
    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    // 4. listen() 함수 호출
    printf("> Calling listen() function...\n");

    listen(server_sock, 5);

    // 5. accept() 함수 호출
    printf("> Calling accept() and waiting for client connection request...\n");

    SOCKADDR_IN client_addr;
    int         sizeof_addr = sizeof(client_addr);
    SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);

    printf(
        "> Client(IP:%s, Port:%d) connected.\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
    );

    // recv() 함수를 통해 연결된 tcp client로부터 데이터 수신
    
    int flag = 0, index = 1;

    while (flag == 0) {
        char buf[BUF_LEN];
        
        int bytes_len = recv(client_sock, buf, BUF_LEN, 0);

        if (bytes_len > 0) { // 데이터를 정상적으로 수신한 경우
            printf("%d> Received %d bytes of data.\n", index, bytes_len);
            index++;
        } else if (bytes_len == SOCKET_ERROR) { // 수신 중 에러 발생 시
            printf("<error> Error occurred during recv(), code(%d)\n", WSAGetLastError());
        } else if (bytes_len == 0) { // 정상 종료 처리 (클라이언트의 연결 종료)
            printf("> Client closed the socket.\n");
            flag = 1;
        }
        Sleep(1000);
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
