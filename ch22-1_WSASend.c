// gcc ch22-1_WSASend.c -o ch22-1_WSASend.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void error_handling(char *msg);

int main(void) {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error!");
	}
	
	// 1. Overlapped 소켓 생성하기
	SOCKET h_socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (h_socket == INVALID_SOCKET) {
		error_handling("WSASocket() error!");
	}
	
	SOCKADDR_IN send_adr;
	memset(&send_adr, 0, sizeof(send_adr));
	send_adr.sin_family         = AF_INET;
	send_adr.sin_addr.s_addr    = inet_addr("127.0.0.1");
	send_adr.sin_port           = htons(9000);

	printf("Client> Connecting...\n");
	if (connect(h_socket, (SOCKADDR*)&send_adr, sizeof(send_adr)) == SOCKET_ERROR) {
		error_handling("connect() error!");
	}
	
	// 2. 이벤트 생성, 등록 및 버퍼 구조체 설정하기
	WSAEVENT ev_obj = WSACreateEvent();
	if (ev_obj == WSA_INVALID_EVENT) {
		error_handling("WSACreateEvent() error!");
	}

	WSAOVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = ev_obj;

	WSABUF data_buf;
	char msg[] = "Network is Computer!";
	data_buf.len = sizeof(msg);
	data_buf.buf = msg;

	printf("Client> Attempting to send data after sleeping for 10 seconds...\n");
	Sleep(10000);

	// 3. Overlapped send 실행
	printf("Client> Sending data...\n");
	DWORD send_bytes = 0;

	if (WSASend(h_socket, &data_buf, 1, &send_bytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			printf("Client> Background data transmission in progress.\n");
			
			// 4. 전송 완료 이벤트 발생 대기하기
			WSAWaitForMultipleEvents(1, &ev_obj, TRUE, WSA_INFINITE, FALSE);
			
			// 5. 이벤트 결과 확인하기 (송신 데이터 크기 확인)
			WSAGetOverlappedResult(h_socket, &overlapped, &send_bytes, FALSE, NULL);
		} else {
			error_handling("WSASend() error");
		}
	}
	printf("Client> Data sending complete. Size: %lu bytes\n", send_bytes);
	
	WSACloseEvent(ev_obj);
	closesocket(h_socket);
	WSACleanup();
	return 0;	
}

void error_handling(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
