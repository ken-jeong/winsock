// gcc midterm/ch02_tcp-client.c -o midterm/ch02_tcp-client.exe -lws2_32

// 클라이언트 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUF_SIZE 100

void ErrorHandling(char* message);

int main(void) {
	printf("> Client started.\n");

	// 1. 윈속 라이브러리 초기화
	WSADATA winsockData;
	int errorNum = WSAStartup(MAKEWORD(2,2), &winsockData);
	if (errorNum != 0)
		ErrorHandling("WSAStartup() error!");

	// 2. 소켓 생성 (IPv4 주소 체계, TCP)
	SOCKET clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
		ErrorHandling("TCP socket creation error");

	SOCKADDR_IN  serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000);  
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 서버에 연결 요청
	errorNum = connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr) );
	printf("> Client requested a TCP connection.\n");

	// 반복해서 send() 함수를 호출하여 서버로 데이터를 전송
	int index;
	char Buffer[MAX_BUF_SIZE];

	// 전송할 데이터 준비
	for (index = 0; index < MAX_BUF_SIZE; index++) {
		Buffer[index] = index;
	}

	for (index = 0; index < 5; index++ ) {
		int returnValue = send(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue == SOCKET_ERROR) {
			printf("<error> Error occurred during send(). code(%d)\n", WSAGetLastError());
		} else if (returnValue > 0) {
			printf("%d> Sent %d bytes via send().\n", index+1, returnValue );
		}
		//
		Sleep(1000);  
	}
	errorNum = closesocket(clientSocket); // 소켓 닫기
	errorNum = WSACleanup(); // 윈속 리소스 정리(해제)
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}