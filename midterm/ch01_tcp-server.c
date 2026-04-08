// gcc midterm/ch01_tcp-server.c -o midterm/ch01_tcp-server.exe -lws2_32

// TCP 서버 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
// #include <ws2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define  MAX_BUF_SIZE  1000

// 에러 처리 함수 선언
void ErrorHandling(char* message);

int main(void) {
	printf("> start TCP server.\n");

	// 윈속 초기화
	WSADATA winsockData;
	WSAStartup(MAKEWORD(2,2), &winsockData);

	// 서버 소켓 생성
	SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 서버 주소 설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_port			= htons(9000); // 포트 번호 9000
	serverAddr.sin_addr.s_addr	= inet_addr("127.0.0.1"); // 루프백 주소
	// InetPton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	// 소켓에 IP 주소와 포트 번호 할당
	bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// 연결 요청 대기 상태로 전환 (대기 큐 크기: 5)
	int backLog = 5; 
	listen(serverSocket, backLog);

	// 클라이언트 연결 요청 수락
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrLen);

	// 데이터 수신 루프
	int stopFlag = 0;
	char Buffer[MAX_BUF_SIZE];
	while(stopFlag == 0) {
		int returnValue = recv(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue > 0) {
			// 데이터 수신 성공 시 수신한 문자열 출력
			Buffer[returnValue] = '\0';
			printf("> recv: %s", Buffer);
		} else if (returnValue == SOCKET_ERROR) {
			// 에러 발생 시 처리
			ErrorHandling("recv() error");
		} else if (returnValue == 0) {
			// 클라이언트가 연결을 종료한 경우 루프 종료
			stopFlag = 1;
		}
	}

	// 소켓 및 윈속 리소스 해제
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}

// 에러 메시지 출력 후 프로그램 종료 함수
void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}