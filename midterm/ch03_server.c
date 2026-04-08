// gcc midterm/ch03_server.c -o midterm/ch03_server.exe -lws2_32

// 서버 프로그램
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define  MAX_BUF_SIZE  1000

void ErrorHandling(char* message);

int main(void) {
	WSADATA winsockData;
	printf("> Server program started.\n");

	// 1. 윈속 초기화... 윈속 라이브러리 로드...
	if (WSAStartup(MAKEWORD(2, 2), &winsockData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 2. 서버 소켓 생성하기...
	SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
		ErrorHandling("TCP socket creation error");

	// 3. bind() 호출하여 소켓에 주소(IP, Port) 할당하기... IPv4
	struct sockaddr_in6 addr;
	SOCKADDR_IN svrAddr;
	memset(&svrAddr, 0, sizeof(svrAddr));
	svrAddr.sin_family = AF_INET; // IPv4 주소 체계
	svrAddr.sin_port = htons(9000); // 호스트 바이트 순서 -> 네트워크 바이트 순서 (short)
	svrAddr.sin_addr.s_addr = inet_addr("1.2.3.4");

	// 4. 소켓에 주소(IP, Port) 바인딩
	int ret = bind(serverSocket, (SOCKADDR*)&svrAddr, sizeof(svrAddr));
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> bind() error. code=%d\n", WSAGetLastError());
		// 서버 소켓 리소스 반환...
		closesocket(serverSocket);
		// 윈속 라이브러리 해제...
		WSACleanup(); 
		return 1;
	}

	// 클라이언트 연결 대기 준비: DB 연결 설정 등...
	// 5. listen: TCP 소켓을 LISTEN(대기) 상태로 변경하기
	ret = listen(serverSocket, 5);
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> listen() error. code=%d\n", WSAGetLastError());
		// 서버 소켓 리소스 반환...
		closesocket(serverSocket);
		// 윈속 라이브러리 해제...
		WSACleanup();
		return 1;
	}

#if 0
	// 1. inet_ntoa() 함수 사용 예제
	char* str_ptr;
	str_ptr = inet_ntoa(svrAddr.sin_addr); 
	printf("IP Address string = %s\n", str_ptr);

	// 2. WSAStringToAddress() 함수 사용 예제
	//    "1.2.3.4:9000" -> IP와 Port를 모두 문자열로 표현
	char* addrStr = "5.6.7.8:9000";
	SOCKADDR_IN addr2;
	int addrSize, ret;
	addrSize = sizeof(addr2);
	ret = WSAStringToAddressA(
			addrStr, // 주소 문자열(IP, Port)
			AF_INET, // IPv4 주소 체계
			NULL,
			(SOCKADDR*)&addr2, // 변환된 주소를 저장할 구조체 포인터
			&addrSize
		);

	if (ret == 0) {
		printf("IP: %#x, Port: %#x \n",
			ntohl(addr2.sin_addr.S_un.S_addr), // Big endian (네트워크 순서) 읽기
			ntohs(addr2.sin_port)	// Big endian (네트워크 순서) 읽기
			);
	}
	// 주소 구조체(addr2) -> 문자열 "5.6.7.8:9000" 로 변환하기
	char addrBuf[50];
	int bufSize;
	bufSize = sizeof(addrBuf);
	ret = WSAAddressToStringA(
			(SOCKADDR*)&addr2, // 변환할 주소 구조체
			sizeof(addr2),
			NULL,
			addrBuf,
			&bufSize
		);
	if (ret == 0) {
		printf("Address string = %s\n", addrBuf);
	}

	// 내 컴퓨터 CPU의 바이트 순서는 무엇일까? (Big/Little endian 확인)
	unsigned long host_addr = 0x12345678; // 호스트 바이트 순서
	unsigned long net_addr = htonl(host_addr); // 빅 엔디안(네트워크 바이트 순서)으로 변환
	printf("> Host byte order: %#x\n", host_addr);
	printf("> Network byte order (Big Endian): %#x\n", net_addr);
#endif

	// 서버 소켓 리소스 반환...
	closesocket(serverSocket);

	// 윈속 라이브러리 해제...
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}