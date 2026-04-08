// gcc midterm/ch07_shutdown-client.c -o midterm/ch07_shutdown-client.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 30

void ErrorHandling(char *message);

int main() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	FILE *fp;
	fp = fopen("receive.dat", "wb");
	SOCKET hSocket = socket(PF_INET, SOCK_STREAM, 0);   

	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	servAdr.sin_port		= htons(9000);

	connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr));
	
	char buf[BUF_SIZE];
	while(1) {
		int readCnt = recv(hSocket, buf, BUF_SIZE, 0);
		if (readCnt != 0) {
			fwrite((void*)buf, 1, readCnt, fp);
		} else {
			printf("Client> server execute shutdown.\n");
			break;
		}
	}
	
	puts("Received file data");
	send(hSocket, "Thank you", 10, 0);

	fclose(fp);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}