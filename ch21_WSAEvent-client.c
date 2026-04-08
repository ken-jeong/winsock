// gcc ch21_WSAEvent-client.c -o ch21_WSAEvent-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024

void error_handling(char *message);

int main() {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) { // Winsock 라이브러리 초기화
		error_handling("WSAStartup() error!");
	}

	SOCKET h_socket = socket(PF_INET, SOCK_STREAM, 0); // TCP 소켓 생성  
	if(h_socket == INVALID_SOCKET) {
		error_handling("socket() error");
	}
	
	// 서버 주소 구조체 초기화 및 설정
	SOCKADDR_IN serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family      = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_adr.sin_port        = htons(9000);
	
	// 서버에 연결 시도
	if (connect(h_socket, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR) {
		error_handling("connect() error!");
	} else {
		puts("Connected...");
	}

	char message[BUF_SIZE];
	// 데이터 송수신 루프
	while (1) {
		fputs("Input message (Q to quit): ", stdout);
		fgets(message, BUF_SIZE, stdin);
		
		// 'q' 또는 'Q' 입력 시 루프 종료
		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n"))
			break;

		// 서버로 메시지 전송
		send(h_socket, message, strlen(message), 0);

		// 서버로부터 메시지 수신
		int str_len = recv(h_socket, message, BUF_SIZE-1, 0);
		message[str_len] = 0;
		printf("Message from server: %s", message);
	}
	
	// 소켓 및 Winsock 리소스 해제
	closesocket(h_socket);
	WSACleanup();
	return 0;
}

// 에러 메시지를 출력하고 프로그램을 종료하는 함수
void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
