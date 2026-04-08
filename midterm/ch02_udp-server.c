// gcc midterm/ch02_udp-server.c -o midterm/ch02_udp-server.exe -lws2_32

// UDP 서버 프로그램
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
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	// UDP 소켓 생성 (SOCK_DGRAM)
	SOCKET servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if(servSock == INVALID_SOCKET)
		ErrorHandling("UDP socket creation error");
	
	// 서버 주소 정보 설정
	SOCKADDR_IN servAdr, clntAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);
	
	// 소켓에 IP 주소와 포트 번호 바인딩
	if(bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	printf("> Waiting UDP message.\n");

	char message[BUF_SIZE];
	int index = 0;

	// 클라이언트로부터 메시지를 수신하기 위한 무한 루프
	while(1) { 
		int clntAdrSz = sizeof(clntAdr);
		// 데이터 수신 (수신 시 클라이언트의 주소 정보가 clntAdr에 저장됨)
		int rcvLen = recvfrom(servSock, message, sizeof(message) - 1, 0,
						   (SOCKADDR*)&clntAdr, &clntAdrSz );
		
		if (rcvLen == SOCKET_ERROR) {
			break;
		}

		message[rcvLen] = 0; // 문자열 끝에 널 문자 삽입
		int strLen = strlen(message);
		
		// 수신한 메시지와 클라이언트 정보 출력
		printf("%d> message from client(IP:%s,PORT:%d) : %s (%d bytes)\n", ++index,
			   inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port), message, strLen );

		Sleep(3000); // 3초 대기
	}	
	
	// 소켓 리소스 해제
	closesocket(servSock);
	WSACleanup();
	return 0;
}

// 에러 처리 및 프로그램 종료 함수
void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}