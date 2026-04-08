// gcc ch12_IOM-select-server.c -o ch12_IOM-select-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 1024

void error_handling(char *msg);

int main(void) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port        = htons(9000);

    bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
    
    listen(server_sock, 5);
    
    fd_set readfds;
    FD_ZERO(&readfds);             // 모든 비트를 0으로 초기화
    FD_SET(server_sock, &readfds); // server_sock을 관찰 대상으로 등록
    
    while (1) { // select() 호출 루프
        // select() 호출 후 값이 변경되는 fd_set과 timeout을 초기화한다.
        fd_set tempfds  = readfds;

        TIMEVAL timeout;
        timeout.tv_sec  = 5;
        timeout.tv_usec = 5000;

        int result = select( // +-< select() >-----------------+
            0,               // | winsock: 0                   |
            &tempfds, 0, 0,  // | readfds, writefds, exceptfds |
            &timeout         // | timeout                      |
        );                   // +------------------------------+

        if (result == 0) {            //  0: timeout
            continue;
        }
        if (result == SOCKET_ERROR) { // -1: SOCKET_ERROR
            break;
        }

        // 이벤트 처리
		for (int i = 0; i < readfds.fd_count; i++) { // 모든 소켓을 순회
			SOCKET sock = readfds.fd_array[i];       // 현재 대상 소켓

			// 1. 이벤트가 발생하지 않은 소켓
			if (!FD_ISSET(sock, &tempfds)) { // sock에 변화가 있으면 양수 반환
				continue;
			}

			// 2. 서버 소켓 이벤트
			if (sock == server_sock) {
                SOCKADDR_IN client_addr;
				int         sizeof_addr = sizeof(client_addr);
				SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);

				FD_SET(client_sock, &readfds); // client_sock을 관찰 대상으로 등록

				printf(
                    "Connected client - Port: %d, IP: %s\n",
					ntohs(client_addr.sin_port), inet_ntoa(client_addr.sin_addr)
                );
				continue; 
			}

			// 3. 클라이언트 소켓 이벤트
            char buf[BUF_LEN];
			int  bytes_len = recv(sock, buf, BUF_LEN-1, 0);

			// 3-1. 연결 종료 요청
			if (bytes_len <= 0) {
				closesocket(sock);
                FD_CLR(sock, &readfds); // sock을 관찰 대상에서 제외
                
				printf(
                    "Closed client - socket: %d, bytes_len: %d\n",
                    sock, bytes_len
                );
				continue;
			}

			// 3-2. 데이터 수신
            SOCKADDR_IN client_addr;
			int         sizeof_addr = sizeof(client_addr);
			getpeername(sock, (SOCKADDR*)&client_addr, &sizeof_addr);
            
			buf[bytes_len] = '\0';
			printf(
                "(Port: %d, IP: %s) Message: %s\n",
				ntohs(client_addr.sin_port), inet_ntoa(client_addr.sin_addr), buf
            );

			send(sock, buf, bytes_len, 0); // echo
		}
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
