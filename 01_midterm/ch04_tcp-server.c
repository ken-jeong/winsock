// gcc midterm/ch04_tcp-server.c -o midterm/ch04_tcp-server.exe -lws2_32

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

    // 3. 구조체에 주소(IP, Port) 설정하기... IPv4
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 루프백 주소
    server_addr.sin_port        = htons(9000); // 호스트 바이트 순서 -> 네트워크 바이트 순서 변환 (short)
    

    // 생성한 서버 소켓에 주소(IP, Port) 바인딩
    result = bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));

    if (result == SOCKET_ERROR) {
        printf("bind() error, code=%d\n", WSAGetLastError());
        closesocket(server_sock); // 소켓 리소스 해제
        WSACleanup(); // 윈속 리소스 해제
        return 1;
    }

    // 서버 동작 전 준비: DB 연결 설정 등

    // 4. listen: TCP 상태를 LISTEN(대기) 상태로 변경하기
    result = listen(server_sock, 5);

    if (result == SOCKET_ERROR) {
        printf("listen() error, code=%d\n", WSAGetLastError());
        // 소켓 리소스 해제...
        closesocket(server_sock);
        // 윈속 리소스 해제...
        WSACleanup();
        return 1;
    }

    // 5. 클라이언트의 연결 요청 처리 준비
    while (1) {
        printf("Server> Waiting for TCP connection request from a new client...\n");

        SOCKADDR_IN client_addr;
        int         sizeof_addr = sizeof(client_addr);
        SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);

        if (client_sock == INVALID_SOCKET) {
            printf("accept() error\n");
            closesocket(server_sock);
            WSACleanup();
            return 1;
        }
        
        // 새로운 클라이언트 연결 요청 수락 완료
        printf("Server> New client (IP: %s, Port: %d) connected.\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port) // 네트워크 바이트 순서(Big endian) -> 호스트 바이트 순서로 변환
        );

        while (1) {
            char buf[BUF_LEN];

            int bytes_len = recv(client_sock, buf, BUF_LEN-1, 0); // < recv() >

            if (bytes_len == 0) { // 클라이언트가 연결을 정상적으로 종료한 경우 (socket close)
                printf("Server> Client disconnected.\n" );
                break;
            } else if (bytes_len == SOCKET_ERROR) { // 에러 발생 시
                printf("Server> recv() SOCKET_ERROR occurred. code=%d\n",
                    WSAGetLastError());
                break;
            } else { // 정상 수신
                buf[bytes_len] = 0; // 수신한 데이터를 문자열로 처리하기 위해 끝에 NULL 추가
                printf(
                    "Server> Received message from client (%d bytes): %s\n",
                    bytes_len, buf
                );
            }
            // 2. 메시지 파싱(parsing)

            // 3. 클라이언트에게 응답 메시지 송신. send() - (Echo 처리)
            bytes_len = send(client_sock, buf, bytes_len, 0);

            if (bytes_len == SOCKET_ERROR) {
                printf("send() error, code=%d\n", WSAGetLastError());
                break;
            } else { // 송신 성공
                printf("Server> Successfully sent message to client (%d bytes).\n", bytes_len);
            }
        }
        closesocket(client_sock);
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
