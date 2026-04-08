// gcc midterm/ch02_tcp-server.c -o midterm/ch02_tcp-server.exe -lws2_32

// 서버 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define  MAX_BUF_SIZE  1000

void ErrorHandling(char* message);

int main(void) {
	printf("> Server program started.\n");

	// 1. 윈속 초기화... 윈속 라이브러리 설정
	WSADATA winsockData;
	if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	// 2. 서버 소켓 생성하기
	SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
		ErrorHandling("TCP socket creation error");

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_port			= htons(9000); 
	serverAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");

	// 3. bind() 함수 호출
	// - 서버에서 사용할 IP, port 설정
	bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// 4. listen() 함수 호출
	printf("> Calling listen() function...\n");
	int backLog = 5; 
	listen(serverSocket, backLog);

	// 5. accept() 함수 호출
	printf("> Calling accept() and waiting for client connection request...\n");
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrLen);

	printf("> Client(IP:%s, Port:%d) connected.\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	 
	// recv() 함수를 통해 연결된 tcp client로부터 데이터 수신
	char Buffer[MAX_BUF_SIZE];
	int stopFlag = 0, index = 1;
	
	while (stopFlag == 0) {
		int returnValue = recv(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue > 0) { // 데이터를 정상적으로 수신한 경우
			printf("%d> Received %d bytes of data.\n", index, returnValue);
			index++;
		} else if (returnValue == SOCKET_ERROR) { // 수신 중 에러 발생 시
			printf("<error> Error occurred during recv(), code(%d)\n", WSAGetLastError());
		} else if (returnValue == 0) { // 정상 종료 처리 (클라이언트의 연결 종료)
			printf("> Client closed the socket.\n"); 
			stopFlag = 1;
		}
		Sleep(1000);
	}
	closesocket(serverSocket); // 서버 소켓 리소스 반환
	WSACleanup(); // 윈속 라이브러리 사용 종료
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}