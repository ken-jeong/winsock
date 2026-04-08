// gcc ch22-1_WSARecv.c -o ch22-1_WSARecv.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024

void error_handling(char *message);

int main(void) {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error!");
	}

	// 1. Overlapped 소켓 생성하기
	SOCKET h_listen_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (h_listen_sock == INVALID_SOCKET) {
		error_handling("WSASocket() error");
	}

	SOCKADDR_IN	listen_addr
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family			= AF_INET;
	listen_addr.sin_addr.s_addr		= inet_addr("127.0.0.1");
	listen_addr.sin_port			= htons(9000);

	if (bind(h_listen_sock, (SOCKADDR*)&listen_addr, sizeof(listen_addr)) == SOCKET_ERROR) {
		error_handling("bind() error");
	}
	if (listen(h_listen_sock, 5) == SOCKET_ERROR) {
		error_handling("listen() error");
	}

	SOCKADDR_IN recv_addr;
	int sizeof_recv_addr = sizeof(recv_addr);
	SOCKET h_recv_sock = accept(h_listen_sock, (SOCKADDR*)&recv_addr, &sizeof_recv_addr);
	if (h_recv_sock == INVALID_SOCKET) {
		error_handling("accept() error");
	}
	printf("Server> accept() completed.\n");

	// --- 시작
	// 2. 이벤트 생성 & 등록, 버퍼 구조체 설정하기
	WSAEVENT ev_obj = WSACreateEvent();
	if (ev_obj == WSA_INVALID_EVENT) {
		error_handling("WSACreateEvent() error");
	}

	WSAOVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = ev_obj;

	WSABUF data_buf;
	char buf[BUF_SIZE];
	data_buf.len = BUF_SIZE;
	data_buf.buf = buf;

	// 3. Overlapped recv 실행
	printf("Server> Calling WSARecv().\n");
	DWORD recv_bytes = 0, flags = 0;

	if (WSARecv(h_recv_sock, &data_buf, 1, &recv_bytes, &flags, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			printf("Server> Background data receiving...\n");
			
			// 4. 이벤트 발생 대기하기
			WSAWaitForMultipleEvents(1, &ev_obj, TRUE, WSA_INFINITE, FALSE);
			
			// 5. 이벤트 결과 확인하기 (수신 데이터 크기 확인)
			WSAGetOverlappedResult(h_recv_sock, &overlapped, &recv_bytes, FALSE, &flags);
			
			printf("Server> Received message: %s\n", buf);
		} else {
			error_handling("WSARecv() error");
		}
	} else {
		// 동기적으로 데이터 수신이 바로 완료되었을 때의 처리
		printf("Server> Received message immediately: %s\n", buf);
	}

	WSACloseEvent(ev_obj);
	// --- 끝

	closesocket(h_recv_sock);
	closesocket(h_listen_sock);
	WSACleanup();
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
