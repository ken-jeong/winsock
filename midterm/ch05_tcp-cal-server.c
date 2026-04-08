// gcc midterm/ch05_tcp-cal-server.c -o midterm/ch05_tcp-cal-server.exe -lws2_32

﻿// ================================================== //
// ch05_tcp-cal-server.c
// ================================================== //
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUF_SIZE  1000

int calculate(int opndNum, int opnds[], char op);
void ErrorHandling(char* message);

int main(void) {
	printf("Server> Run program\n");
	WSADATA wsaData;									// 라이브러리 정보
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {	// (2.2 버전)
		ErrorHandling("Server> WSAStartup() error");
	} else {
		printf("Server> WSAStartup() success\n");
	}

	// ================================================== //
	// 1. socket(): 서버 소켓 생성
	// ================================================== //
	SOCKET serverSocket = socket(
		PF_INET,		// IPv4
		SOCK_STREAM,	// TCP
		0				// IPPROTO_TCP
	);

	if (serverSocket == INVALID_SOCKET) {
		ErrorHandling("Server> socket() INVALID_SOCKET error");
	} else {
		printf("Server> socket() success\n");
	}

	// ================================================== //
	// 2. bind(): 서버 소켓에 주소 할당
	// ================================================== //
	SOCKADDR_IN servAddr;								// 주소표현 구조체
	memset(&servAddr, 0, sizeof(servAddr));				// 메모리 공간을 0으로 초기화

	servAddr.sin_family = AF_INET;						// 1. IPv4
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// 2. IP 주소(Loopback): 32비트 정수형(빅 엔디안)으로 변환
	servAddr.sin_port = htons(9000);					// 3. Port: 호스트 순서 to 네트워크 순서 (short)

	int ret = bind(
		serverSocket,			// 소켓
		(SOCKADDR*)&servAddr,	// 주소
		sizeof(servAddr)		// sizeof(주소)
	);
	
	if (ret == SOCKET_ERROR) {
		printf("Server> bind() SOCKET_ERROR, code=%d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	} else {
		printf("Server> Binded to client successfully.\n");
	}

	// (서비스 제공 준비: DB 서버 연결 등)

	// ================================================== //
	// 3. listen(): 클라이언트 소켓의 연결 요청 대기
	// ================================================== //
	ret = listen(
		serverSocket,	// 소켓
		5				// 큐
	);
	if (ret == SOCKET_ERROR) {
		printf("Server> listen() SOCKET_ERROR, code=%d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// client의 연결 요청을 처리해야 함 -> table 앉힘-주문 받기
	SOCKET ClientSock;
	SOCKADDR_IN clientAddr; // 주소표현 구조체
	int clientAddrLen = sizeof(clientAddr);

	char opndCnt;
	char message[MAX_BUF_SIZE];

	while (1) { // 여러 client를 loop 돌면서 반복해서 처리하기 위한 loop
		printf("Server> Waiting for client connection requests...\n");

		// ================================================== //
		// 4. accept(): 클라이언트 소켓의 연결 요청 수락
		// ================================================== //
		ClientSock = accept(
			serverSocket,			// 소켓
			(SOCKADDR*)&clientAddr,	// 주소
			&clientAddrLen			// 주소 크기
		);

		if (ClientSock == INVALID_SOCKET) {
			printf("Server> accept() INVALID_SOCKET error\n");
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}

		// 새로운 client 연결 요청을 위한 소켓 생성 완료
		printf("Server> 새로운 client(IP:%s, Port:%d)가 연결되었습니다.\n",
			inet_ntoa(clientAddr.sin_addr),	// Network to ASCII
			ntohs(clientAddr.sin_port)		// 네트워크 순서 to 호스트 순서 (short)
		);

		int flag = 1;
		while (flag) { // 특정 연결된 client (Accept된 Client)의 여러 요청 반복 처리
			// ================================================== //
			// 5.1. recv(): 계산할 데이터를 클라이언트 소켓으로부터 수신
			// ================================================== //
			// 메시지의 고정 부분(opndCnt, 1byte) 읽음 (recv -1)
			ret = recv(		// 수신된 메시지의 첫 번째 바이트 읽기
				ClientSock,	// 소켓
				&opndCnt,	// 버퍼
				1,			// 버퍼 크기
				0			// 플래그
			); 

			if (ret == 0) { // 상대가 socket을 close한 상태
				printf("Server> The client closed the socket.\n");
				break;
			} else if (ret == SOCKET_ERROR) {
				printf("Server> recv() SOCKET_ERROR, code=%d\n", WSAGetLastError());
				break;
			}

			// 1 바이트 수신한 상황...피연산자의 갯수 message[0]에 저장된 상태
			// opndCnt = message[0];

			// 1.2. 메시지의 나머지 부분을 마저 읽음 (recv -2)
			int rcvSum = 0; // 읽은 누적치
			int rcvTarget = (int)opndCnt * sizeof(int) + 1; // 읽어야될 메시지의 크기가 정해짐

			// recv loop 원하는 크기가 읽혀질 때까지 반복 recv 수행
			while (rcvSum < rcvTarget) {
				ret = recv(
					ClientSock,			// 소켓
					&message[rcvSum],	// 버퍼
					rcvTarget - rcvSum,	// 버퍼 크기
					0					// 플래그
				);

				if (ret == 0) { // 상대가 socket close 한 상태
					printf("Server> The client closed the socket.\n");
					flag = 0;
					break;
				} else if (ret == SOCKET_ERROR) {
					printf("Server> recv() SOCKET_ERROR, code=%d\n", WSAGetLastError());
					flag = 0;
					break;
				}
				rcvSum += ret;
			}

			// ================================================== //
			// 계산하기
			// ================================================== //
			int result = calculate(opndCnt, message, message[rcvTarget - 1]);

			// ================================================== //
			// 5.2. send(): 계산한 데이터를 클라이언트 소켓으로 송신
			// ================================================== //
			ret = send(			// 결과 send
				ClientSock,		// 소켓
				(char*)&result,	// 버퍼
				sizeof(result),	// 버퍼 크기
				0				// 플래그
			);

			if (ret == SOCKET_ERROR) {
				printf("Server> send() SOCKET_ERROR, code = %d\n", WSAGetLastError());
				break;
			} else { // send OK
				printf("Server> sent result to client %d bytes.\n", ret);
			}
		}
		closesocket(ClientSock); // 클라이언트 소켓 종료
	}

	// ================================================== //
	// 6. closesocket(): 소켓 종료
	// ================================================== //
	closesocket(serverSocket);	// 서버 소켓 종료
	WSACleanup();				// 라이브러리 종료
	return 0;
}

int calculate(int opndNum, int opnds[], char op) {
	int result = opnds[0];

	switch (op) {
	case '+': // p1, p2, p3 -> result = p1 + p2 + p3;
		for (int i = 1; i < opndNum; i++) {
			result += opnds[i];
		}
		break;

	case '-': // p1, p2, p3 -> result = p1 - p2 - p3;		
		for (int i = 1; i < opndNum; i++) {
			result -= opnds[i];
		}
		break;

	case '*': // p1, p2, p3 -> result = p1 * p2 * p3;		
		for (int i = 1; i < opndNum; i++) {
			result *= opnds[i];
		}
		break;
	}

	return result;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}