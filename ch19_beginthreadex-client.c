// gcc ch19_beginthreadex-client.c -o ch19_beginthreadex-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_PACKET_SIZE  120

void error_handling(char *message);

int main(void) {
	WSADATA wsa_data;
	if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		error_handling("WSAStartup() error!"); 

	SOCKET client_socket = socket(PF_INET, SOCK_STREAM, 0);   
	if(client_socket == INVALID_SOCKET)
		error_handling("socket() error");
	
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(9000);
	 
	// TCP 연결 요청
	int ret = connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> Client. connect() failed.\n");
		closesocket(client_socket);
		printf("Client> close socket...\n");
		WSACleanup();
		return 0;
	} else {
		printf("Client> connection established...\n");
	}
	
	// 1. 계산할 피연산자 및 연산자를 입력받고 서버로 전송한 뒤 결과 수신
	char msg_buffer[MAX_PACKET_SIZE];
	int operand_count, received_sum, receive_total, result, is_active;
	is_active = 1;
	while (is_active) {
		// 1. 사용자로부터 계산 자료 입력... (피연산자 개수, 피연산자, 연산자)		
		printf("Client: Enter the number of operands: ");
		scanf("%d", &operand_count);
		msg_buffer[0] = (char)operand_count;
		for (int i = 0; i < operand_count; i++) {
			printf("- Enter operand %d: ", i+1);
			scanf("%d", &msg_buffer[1+i*sizeof(int)]);
		}
		rewind(stdin);
		
		printf("> Enter operator: ");
		scanf("%c", &msg_buffer[1+operand_count*sizeof(int)]);
		send(client_socket, msg_buffer, 2+operand_count*sizeof(int), 0);

		// 2. 결과 수신 및 출력		
		received_sum = 0; // 현재 수신된 바이트 수
		receive_total = sizeof(result); // 수신 목표 바이트 수

		while(received_sum < receive_total) {
			ret = recv(client_socket, &msg_buffer[received_sum], receive_total-received_sum, 0);
			if (ret <= 0) {
				printf("<ERROR> recv failed.\n");
				is_active = 0;
				break;
			} else {
				received_sum = received_sum + ret;
				printf("Client> recv %d bytes. sum=%d, total=%d\n", ret, received_sum, receive_total);
			}
		}

		if (is_active == 1) {
			result = *((int*)msg_buffer);
			printf("Client> Result received from server = %d\n", result);
		}
	}
	closesocket(client_socket);
	printf("Client> close socket...\n");
	WSACleanup();
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}