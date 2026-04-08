// gcc midterm/ch07_shutdown-server.c -o midterm/ch07_shutdown-server.exe -lws2_32

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
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	FILE *fp;
	fp = fopen("file_server_win.c", "rb"); 
	SOCKET hServSock = socket(PF_INET, SOCK_STREAM, 0);   
	
	SOCKADDR_IN servAdr, clntAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);
	
	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);
	
	int clntAdrSz = sizeof(clntAdr);    
	SOCKET hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);
	
	char buf[BUF_SIZE];
	while(1) {
		int readCnt = fread((void*)buf, 1, BUF_SIZE, fp);
		if (readCnt < BUF_SIZE) {
			send(hClntSock, (char*)&buf, readCnt, 0);
			break;
		}
		send(hClntSock, (char*)&buf, BUF_SIZE, 0);
	}
	
	shutdown(hClntSock, SD_SEND);
	recv(hClntSock, (char*)buf, BUF_SIZE, 0);
	printf("Message from client: %s \n", buf);
	 
	fclose(fp);
	closesocket(hClntSock);
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}