// gcc midterm/ch05_tcp-cal-client.c -o midterm/ch05_tcp-cal-client.exe -lws2_32

﻿// ================================================== //
// ch05_tcp-cal-client.c
// ================================================== //
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1000

void ErrorHandling(char* message);

int main() {
	printf("Client> Run program\n");
	WSADATA wsaData;									// 라이브러리 정보
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {	// (2.2 버전)
		ErrorHandling("Client> WSAStartup() error");
	} else {
		printf("Client> WSAStartup() success\n");
	}

	// ================================================== //
	// 1. socket(): 클라이언트 소켓 생성
	// ================================================== //
	SOCKET hSocket = socket(
		PF_INET,		// IPv4
		SOCK_STREAM,	// TCP
		0				// IPPROTO_TCP
	);

	if (hSocket == INVALID_SOCKET) {
		printf("Client> socket() INVALID_SOCKET error\n");
		WSACleanup();
		return 1;
	} else {
		printf("Client> socket() success\n");
	}
	
	// ================================================== //
	// 2. connect(): 서버 소켓에 연결 요청
	// ================================================== //
	SOCKADDR_IN servAddr;									// 주소표현 구조체
	memset(&servAddr, 0, sizeof(servAddr));					// 메모리 공간을 0으로 초기화

	servAddr.sin_family = AF_INET;							// 1. IPv4
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");	// 2. IP 주소(Loopback): 32비트 정수형(빅 엔디안)으로 변환
	servAddr.sin_port = htons(9000);						// 3. Port: 호스트 순서 to 네트워크 순서 (short)

	int ret = connect(
		hSocket,				// 소켓
		(SOCKADDR*)&servAddr,	// 주소
		sizeof(servAddr)		// sizeof(주소)
	);

	if (ret == SOCKET_ERROR) {
		printf("Client> connect() SOCKET_ERROR, code=%d\n", WSAGetLastError());
		closesocket(hSocket);
		WSACleanup();
		return 1;
	} else {
		printf("Client> Connected to server successfully.\n");
	}

	int opndCnt;
	char message[BUF_SIZE]; // char 배열 message[]에 저장

	while (1) {
		// ================================================== //
		// 계산할 데이터를 입력
		// ================================================== //
		printf("Client> Enter the number of operands: ");		// 피연산자 수 입력
		scanf("%d", &opndCnt);									// 저장 위치
		message[0] = opndCnt;									// 저장 위치

		for (int i = 0; i < opndCnt; i++) {						// 피연산자 수만큼
			printf("Client> Enter the %dth operand: ", i + 1);	// 피연산자 입력
			scanf("%d", &message[1 + sizeof(int) * i]);			// 저장 위치
		}
		getchar();												// 이전 엔터 삭제

		printf("Client> Enter the operator(+/-/*): ");			// 연산자 입력
		scanf("%c", &message[1 + sizeof(int) * opndCnt]);		// 저장 위치

		// ================================================== //
		// 3.1. send(): 계산할 데이터를 서버 소켓으로 송신
		// ================================================== //
		ret = send(
			hSocket,						// 소켓
			message,						// 버퍼
			1 + sizeof(int) * opndCnt + 1,	// 버퍼 크기: 피연산자 수(1) + 피연산자 + 연산자(1)
			0								// 플래그
		);

		if (ret == SOCKET_ERROR) {
			printf("Client> send() SOCKET_ERROR, code = %d\n", WSAGetLastError());
			break;
		} else { // send() OK
			printf("Client> sent to server %d bytes.\n", ret);
		}

		// ================================================== //
		// 3.2. recv(): 계산한 데이터를 서버 소켓으로부터 수신
		// ================================================== //
		int rcvSum = 0;		// 수신 누적치: 현재까지 수신량을 누적
		int rcvTarget = 4;	// 수신 목표치: result 정수 4B 수신 예정

		while (rcvSum < rcvTarget) {
			ret = recv(
				hSocket,					// 소켓
				&message[rcvSum],			// 버퍼
				sizeof(message) - rcvSum,	// 버퍼 크기
				0							// 플래그
			);

			if (ret == 0) {	// 상대가 socket을 close한 상태
				printf("Client> The server closed the socket.\n");
				closesocket(hSocket);
				WSACleanup();
				return 1;
			} else if (ret == SOCKET_ERROR) {
				printf("Client> recv() SOCKET_ERROR\n");
				closesocket(hSocket);
				WSACleanup();
				return 1;
			} else {
				rcvSum += ret;
			}
		}

		// 결과 수신 완료
		int result = *((int*)message);
		printf("Client> Calculation result received from the server: %d\n", result);
	}

	// ================================================== //
	// 4. closesocket(): 소켓 종료
	// ================================================== //
	closesocket(hSocket);	// 소켓 종료
	WSACleanup();			// 라이브러리 종료
	return 0;
}

void ErrorHandling(char* message) {
	printf("[ERROR] %s\n", message);
	exit(1);
}