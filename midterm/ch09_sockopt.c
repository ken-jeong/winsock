// gcc midterm/ch09_sockopt.c -o midterm/ch09_sockopt.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void ErrorHandling(char* message);

int main(void) {
	WSADATA wsaData;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	SOCKET hSock = socket(PF_INET, SOCK_STREAM, 0);

	int a;
	int b = sizeof(a);

	// 1. 송신 버퍼 크기 확인
	int ret = getsockopt(hSock, SOL_SOCKET, SO_SNDBUF, (char*)&a, &b);
	printf("1> Send buffer size: %d bytes\n", a);

	// 2. 송신 버퍼 크기를 2배로 설정
	a = a * 2; // 2배로 설정
	ret = setsockopt(hSock, SOL_SOCKET, SO_SNDBUF, (char*)&a, sizeof(a));

	// 3. 변경 후 송신 버퍼 크기 재확인
	b = sizeof(a);
	ret = getsockopt(hSock, SOL_SOCKET, SO_SNDBUF, (char*)&a, &b);
	printf("2> Send buffer size: %d bytes\n", a);
	
	closesocket(hSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}