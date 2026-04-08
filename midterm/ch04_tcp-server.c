// gcc midterm/ch04_tcp-server.c -o midterm/ch04_tcp-server.exe -lws2_32

// TCP 서버 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define  MAX_BUF_SIZE  1000

void ErrorHandling(char* message);

int main(void) {
	printf("> Server program started.\n");

	// 1. 윈속 초기화... 윈속 라이브러리 사용 설정...
	WSADATA winsockData;
	if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 2. 서버 소켓 생성하기...
	SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
		ErrorHandling("TCP socket creation error");

	// 3. 구조체에 주소(IP, Port) 설정하기... IPv4
	SOCKADDR_IN svrAddr;
	memset(&svrAddr, 0, sizeof(svrAddr));
	svrAddr.sin_family = AF_INET; // IPv4
	svrAddr.sin_port = htons(9000); // 호스트 바이트 순서 -> 네트워크 바이트 순서 변환 (short)
	svrAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 루프백 주소

	// 생성한 서버 소켓에 주소(IP, Port) 바인딩
	int ret = bind(serverSocket, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> bind() error. code=%d\n", WSAGetLastError());
		closesocket(serverSocket); // 소켓 리소스 해제
		WSACleanup(); // 윈속 리소스 해제
		return 1;
	}

	// 서버 동작 전 준비: DB 연결 설정 등

	// 4. listen: TCP 상태를 LISTEN(대기) 상태로 변경하기
	ret = listen(serverSocket, 5);
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> listen() error. code=%d\n", WSAGetLastError());
		// 소켓 리소스 해제...
		closesocket(serverSocket);
		// 윈속 리소스 해제...
		WSACleanup();
		return 1;
	}

	// 5. 클라이언트의 연결 요청 처리 준비
	SOCKET ClientSock;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1) {
		printf("Server> Waiting for TCP connection request from a new client...\n");
		ClientSock = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
		if (ClientSock == INVALID_SOCKET) {
			printf("Server> accept error.\n");
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}

		// 새로운 클라이언트 연결 요청 수락 완료
		printf("Server> New client (IP: %s, Port: %d) connected.\n",
			inet_ntoa(clientAddr.sin_addr),
			ntohs(clientAddr.sin_port) // 네트워크 바이트 순서(Big endian) -> 호스트 바이트 순서로 변환
		);

		while (1) {
			// 1. 클라이언트의 요청 메시지 수신. recv()
			char message[MAX_BUF_SIZE];
			int strLen = recv(ClientSock, message, MAX_BUF_SIZE-1, 0);
			if (strLen == 0) { // 클라이언트가 연결을 정상적으로 종료한 경우 (socket close)
				printf("Server> Client disconnected.\n" );
				break;
			} else if (strLen == SOCKET_ERROR) { // 에러 발생 시
				printf("Server> recv() SOCKET_ERROR occurred. code=%d\n", 
					WSAGetLastError());
				break;
			} else { // 정상 수신
				message[strLen] = 0; // 수신한 데이터를 문자열로 처리하기 위해 끝에 NULL 추가
				printf("Server> Received message from client (%d bytes): %s\n",
					strLen, message);
			}
			// 2. 메시지 파싱(parsing)

			// 3. 클라이언트에게 응답 메시지 송신. send() - (Echo 처리)
			ret = send(ClientSock, message, strLen, 0);
			if (ret == SOCKET_ERROR) {
				printf("<ERROR> send() error. code=%d\n", WSAGetLastError());
				break;
			} else { // 송신 성공
				printf("Server> Successfully sent message to client (%d bytes).\n", ret);
			}
		}
		closesocket(ClientSock); // 클라이언트 소켓 닫기
	}
	closesocket(serverSocket); // 서버 소켓 리소스 해제
	WSACleanup(); // 윈속 리소스 해제
	return 0;
}

void ErrorHandling(char* message) { // 에러 처리 함수
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}