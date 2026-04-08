// gcc midterm/ch04_tcp-client.c -o midterm/ch04_tcp-client.exe -lws2_32

// 클라이언트 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1000

void ErrorHandling(char* message);

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 1. 클라이언트 소켓 생성하기
	SOCKET hSocket = socket(PF_INET, SOCK_STREAM, 0); // TCP
	if (hSocket == INVALID_SOCKET) {
		printf("Client> socket() error.\n");
		WSACleanup();
		return 1;
	}

	// 2. 서버로 TCP 연결 시도하기
	SOCKADDR_IN servAddr; 
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET; // IPv4
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(9000); 

	int ret = connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr));
	if (ret == SOCKET_ERROR) {
		printf("Client> connect() error, code=%d\n", WSAGetLastError());
		closesocket(hSocket);
		WSACleanup();
		return 1;
	}

	// 서버와 TCP 연결 성공
	printf("Client> Connected to the server successfully.\n");
	int flag = 1;
	char message[BUF_SIZE];

	while (flag) {
		// 1. 사용자로부터 문자열 입력 받기 (q 입력 시 종료)
		printf("Client> Enter message (q to quit): ");
		scanf("%s", message);

		// "q" 입력 시 처리하는 부분
		if (!strcmp(message, "q")) {
			// 사용자가 "q"를 입력하면 루프 탈출
			break;
		}
		
		// 2. 입력된 문자열을 서버로 전송. send()
		ret = send(hSocket, message, strlen(message), 0);
		if (ret == SOCKET_ERROR) {
			printf("<ERROR> send() error. code = %d\n", WSAGetLastError());
			break;
		}
		else { // 전송 성공
			printf("Client> Message sent successfully (%d bytes).\n", ret);
		}

		// 3. 서버로부터 문자열을 수신하여 출력. recv()
		// TCP recv 특성 고려: 
		// - 보낸 데이터를 한 번에 다 받지 못할 수도 있음
		// ---> 데이터를 모두 수신하기 위해 recv 루프를 돌아야 함
		
		int rcvTotal = ret; // 수신 목표치 (보낸 만큼 받아야 함, 에코 서버 기준)
		int rcvSum = 0;		// 수신 누적치, 0으로 초기화
		int strLen;

		while (rcvSum < rcvTotal) {
			strLen = recv(hSocket, &message[rcvSum], BUF_SIZE - 1, 0);
			if (strLen == 0) { // 서버가 연결을 종료한 경우
				printf("Client> Server closed the connection.\n");
				flag = 0;
				break;
			}
			else { // 수신 성공
				message[strLen] = 0; // 문자열의 끝(NULL) 처리
				printf("Client> Received data from server (%d bytes): %s\n",
					strLen, message);
			}
			rcvSum += strLen;
			message[rcvSum] = 0; // 누적된 문자열의 끝 표시
			printf("Client> Message from server: %s (rcvSum:%d, rcvTotal:%d)\n",
				message, rcvSum, rcvTotal
			);
		}	
	}
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	printf("[ERROR] %s \n", message);
	exit(1);
}