// gcc "ch22-2_ CompletionRoutineRecv.c" -o "ch22-2_ CompletionRoutineRecv.exe" -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024

void CALLBACK comp_routine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void error_handling(char *message);

WSABUF  data_buf;
char    buf[BUF_SIZE];
DWORD   recv_bytes = 0;

int main() {
	WSADATA         wsa_data;
	SOCKET          h_lisn_sock, h_recv_sock;	
	SOCKADDR_IN     lisn_adr, recv_adr;
	WSAOVERLAPPED   overlapped;
	WSAEVENT        ev_obj;
	DWORD           idx, flags = 0;
	int             recv_adr_sz;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error!");
	}

	// 1. Overlapped 소켓 생성하기
	h_lisn_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (h_lisn_sock == INVALID_SOCKET) {
		error_handling("WSASocket() error");
	}
	
	memset(&lisn_adr, 0, sizeof(lisn_adr));
	lisn_adr.sin_family      = AF_INET;
	lisn_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
	lisn_adr.sin_port        = htons(9000);

	if (bind(h_lisn_sock, (SOCKADDR*)&lisn_adr, sizeof(lisn_adr)) == SOCKET_ERROR) {
		error_handling("bind() error");
	}
	if (listen(h_lisn_sock, 5) == SOCKET_ERROR) {
		error_handling("listen() error");
	}

	recv_adr_sz = sizeof(recv_adr);    
	h_recv_sock = accept(h_lisn_sock, (SOCKADDR*)&recv_adr, &recv_adr_sz);
	if (h_recv_sock == INVALID_SOCKET) {
		error_handling("accept() error");
	}
	printf("Server> accept()\n");

	// 2. Overlapped 구조체 초기화 및 버퍼 구조체 설정하기
	memset(&overlapped, 0, sizeof(overlapped));
	data_buf.len = BUF_SIZE;
	data_buf.buf = buf;

	// 3. 이벤트 생성하기 (더미 이벤트 객체)
	ev_obj = WSACreateEvent();
	if (ev_obj == WSA_INVALID_EVENT_PARAMETER) {
		error_handling("WSACreateEvent() error");
	}
	
	printf("Server> recv().\n");
	if (WSARecv(h_recv_sock, &data_buf, 1, &recv_bytes, &flags, &overlapped, comp_routine) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			printf("Server> Background data receive...\n");
		}
	}

	// 4. 이벤트 발생 대기하기 (Alertable Wait 상태 진입)
	printf("Server> enter alertable wait.\n");
	idx = WSAWaitForMultipleEvents(1, &ev_obj, FALSE, WSA_INFINITE, TRUE);
	
	if (idx == WAIT_IO_COMPLETION) {
		printf("Server> Overlapped I/O Completed...\n");
	} else {    // 에러가 발생한 경우
		error_handling("WSARecv() error");
	}

	WSACloseEvent(ev_obj);
	closesocket(h_recv_sock);
	closesocket(h_lisn_sock);
	WSACleanup();
	return 0;
}

void CALLBACK comp_routine(DWORD dw_error, DWORD sz_recv_bytes, LPWSAOVERLAPPED lp_overlapped, DWORD flags) {
	if (dw_error != 0) {
		error_handling("Comp> CompRoutine error");
	} else {
		recv_bytes = sz_recv_bytes;
		printf("Comp> Received message: %s \n", buf);
	}
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
