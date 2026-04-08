// gcc midterm/ch06_bound-server.c -o midterm/ch06_bound-server.exe -lws2_32

// Setup Program
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 30

void ErrorHandling(char *message);

int main() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!"); 
	}

	// ================================================== //
	// 1. socket(): 서버 소켓 생성
	// ================================================== //
	SOCKET servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if(servSock == INVALID_SOCKET) {
		ErrorHandling("UDP socket creation error");
	}
	
	// ================================================== //
	// 2. bind(): 서버 소켓에 주소 할당
	// ================================================== //
	SOCKADDR_IN servAdr, clntAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(9000);
	if(bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling("bind() error");
	}

	int clntAdrSz;
	int strLen;
	char message[BUF_SIZE];

	while(1) {		
		clntAdrSz = sizeof(clntAdr);
		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);

		message[strLen] = '\0';
		printf("1. received msg(%s) from(IP:%s,PORT:%d)\n", message, 
			    inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

		printf("Server> Sleep 3 sec\n");
		Sleep(3000);

		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
		message[strLen] = '\0';
		printf("2. received msg(%s) from(IP:%s,PORT:%d)\n", message, 
			    inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

		printf("Server> Sleep 3 sec\n");
		Sleep(3000);

		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
		message[strLen] = '\0';
		printf("3. received msg(%s) from(IP:%s,PORT:%d)\n", message, 
			    inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));		
	}

	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}