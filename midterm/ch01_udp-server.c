// gcc midterm/ch01_udp-server.c -o midterm/ch01_udp-server.exe -lws2_32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 100

void ErrorHandling(char *message);

int main(void) {
	WSADATA			wsaData;
	SOCKET			servSock;
	char			message[BUF_SIZE];
	int				strLen;
	int				clntAdrSz;
	SOCKADDR_IN		servAdr, clntAdr;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}
	
	servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (servSock == INVALID_SOCKET) {
		ErrorHandling("UDP socket creation error");
	}
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);
	
	if (bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling("bind() error");
	}

	printf("> start UDP server.\n");
	
	clntAdrSz = sizeof(clntAdr);
	strLen = recvfrom( servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
	message[strLen] = '\0';
	printf("> message from client: %s\n", message);
	
	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}