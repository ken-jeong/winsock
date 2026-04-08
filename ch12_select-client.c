// gcc ch12_select-client.c -o ch12_select-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h> // exit() 함수 사용을 위해 추가
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 512

int recv_n(SOCKET sock, char *buf, int len, int flags);
void error_display(const char *str);

int main(void) {
	WSADATA wsa;
	int ret_val = WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		error_display("socket() error(INVALID_SOCKET)");
	}

	// connect() 함수를 이용하여 서버에 연결 시도
	SOCKADDR_IN	server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(9000);
	
	ret_val = connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (ret_val == SOCKET_ERROR) {
		error_display("connect() error(SOCKET_ERROR)");
	}

	char buf[BUF_SIZE + 1];

	while (1) {
		ZeroMemory(buf, sizeof(buf));

		// 사용자로부터 보낼 데이터 입력
		printf("\n[Data to Send] ");
		if (fgets(buf, BUF_SIZE + 1, stdin) == NULL) {
			break;
		}

		ret_val = send(client_socket, buf, strlen(buf), 0);
		if (ret_val == SOCKET_ERROR) {
			printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
			break;
		}
		printf("[TCP Client] Sent %d bytes.\n", ret_val);

		// 서버로부터 데이터 수신
		ret_val = recv_n(client_socket, buf, ret_val, 0);
		if (ret_val == SOCKET_ERROR) {
			printf("<ERROR> recv_n()(SOCKET_ERROR)!!!\n");
			break;
		} else if (ret_val == 0) {
			break;
		}

		// 수신한 데이터 출력
		buf[ret_val]= '\0';
		printf("[TCP Client] Received %d bytes.\n", ret_val);
		printf("[Received Data] %s", buf);
	}
	closesocket(client_socket);
	WSACleanup();
	return 0;
}

// 지정한 크기(len)만큼 데이터를 읽어들이는 사용자 정의 수신 함수
int recv_n(SOCKET sock, char *buf, int len, int flags) {
	int left = len;
	int received;
	char *ptr = buf;
	
	while (left > 0) {
		received = recv(sock, ptr, left, flags);
		if (received == SOCKET_ERROR) {
			return SOCKET_ERROR;
		} else if (received == 0) {
			break;
		}
		left -= received;
		ptr += received;
	}
	return (len - left);
}

void error_display(const char* str) {
	printf("<ERROR> %s!!!\n", str);
	exit(-1);
}