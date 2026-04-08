// gcc ch19_beginthreadex-server.c -o ch19_beginthreadex-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_PACKET_SIZE  120

unsigned WINAPI process_client(LPVOID arg);
int calculate(int operand_count, int data[], char op);
void handle_error(char *message);

int main(void) {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		handle_error("WSAStartup() error!"); 
	
	SOCKET server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET)
		handle_error("socket() error");
	
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 모든 IP 허용 시 htonl(INADDR_ANY) 사용
	server_addr.sin_port = htons(9000);

	if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		handle_error("bind() error");
	
	// 클라이언트 접속 대기 준비 완료
	listen(server_socket, 3);

	while (1) {
		printf("Server> Waiting for client connection request...\n");

		SOCKADDR_IN client_addr;
		int client_addr_size = sizeof(client_addr);
		SOCKET client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);

		if (client_socket == -1) {
			printf("<ERROR> accept() failed.\n");
		} else {
			printf("Server> Connected to client (IP: %s, Port: %d).\n",
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		}

		// 스레드 생성
		DWORD thread_id;
		HANDLE hThread = _beginthreadex(
			NULL,				  // 보안 정보
			0,					  // 스택 크기
			process_client,		  // 스레드가 실행할 함수명
			(LPVOID)client_socket,// 파라미터
			0,					  // 플래그값 (0: 스레드 생성 후 바로 실행 상태로 시작)
			&thread_id			  // 스레드 ID값
		);
		printf("Server> New thread created. Thread ID: %d\n", thread_id);
	}
	closesocket(server_socket);
	WSACleanup();
	return 0;
}

unsigned WINAPI process_client(LPVOID arg) {
	SOCKET client_socket = (SOCKET)arg;
	int is_running = 1;

	while (is_running) {
		printf("Thread> Waiting for calculation request...\n");

		char operand_count, message[MAX_PACKET_SIZE];
		recv(client_socket, &operand_count, sizeof(operand_count), 0);
		printf("Thread> Operand count = %d\n", operand_count);

		int received_bytes = 0; // 현재까지 수신한 데이터 크기 (바이트 단위)
		int total_bytes = operand_count * sizeof(int) + 1; // 총 수신해야 할 목표 데이터 크기 (피연산자들 + 연산자 1바이트)
		
		while (received_bytes < total_bytes) {
			int ret = recv(client_socket, &message[received_bytes], total_bytes - received_bytes, 0);
			if (ret <= 0) {
				printf("<ERROR> recv() failed or connection closed.\n");
				is_running = 0;
				break;
			} else {
				received_bytes = received_bytes + ret;
				printf("Thread> Received %d bytes. sum=%d, total=%d\n", ret, received_bytes, total_bytes);
			}
		}
		
		if (is_running == 1) {
			// 1. 연산 수행
			int result = calculate((int)operand_count, (int*)message, message[total_bytes - 1]);
			printf("Thread> Calculation result = %d.\n", result);
			
			// 2. 클라이언트에게 연산 결과 전송
			send(client_socket, (char*)&result, sizeof(result), 0);
			printf("Thread> Sent the result to the client.\n");
		}
	}
	printf("Thread> Closed socket with the client.\n");
	closesocket(client_socket);
	return 0;
}

// 클라이언트가 보낸 데이터를 바탕으로 사칙연산을 수행하는 함수
int calculate(int operand_count, int data[], char op) {
	int i;
	int result = data[0];

	switch (op) {
	case '+':
		for (i = 1; i < operand_count; i++) {
			result = result + data[i];
		}
		break;
	case '-':
		for (i = 1; i < operand_count; i++) {
			result = result - data[i];
		}
		break;
	case '*':
		for (i = 1; i < operand_count; i++) {
			result = result * data[i];
		}
		break;
	}
	return result;
}

/*
	10, 5, 2, 연산자 -이면 결과는 3
*/

// 에러 발생 시 메시지 출력 후 프로그램을 종료하는 함수
void handle_error(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}