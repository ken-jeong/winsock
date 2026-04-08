// gcc midterm/ch02_udp-client.c -o midterm/ch02_udp-client.exe -lws2_32

// 클라이언트 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 100

// 에러 처리 함수 선언
void ErrorHandling(char *message);
 
int main(int argc, char *argv[]) {
	WSADATA wsaData;
	// 윈속 라이브러리 초기화
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	// UDP 소켓 생성
	SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	// 서버 주소 정보 설정
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	servAdr.sin_port		= htons(9000);
	
	// 송신할 메시지 초기화
	char message[BUF_SIZE] = "abcdefghijklmnopqrstuvwxyz";
	
	// 1초 간격으로 메시지를 총 5회 전송
	for (int index = 0; index < 5; index++) {
		int strLen = strlen(message);
		sendto(sock, message, strLen, 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
		
		// 콘솔 출력 (영어)
		printf("%d> sent UDP message:%s\n", index+1, message);

		Sleep(1000); // 1초 대기
	}	 
	
	// 소켓 및 윈속 리소스 정리
	closesocket(sock);
	WSACleanup();
	return 0;
}

// 에러 처리 함수 정의
void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}