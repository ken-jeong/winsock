// gcc ch14-1_multicast-recvfrom.c -o ch14-1_multicast-recvfrom.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> // for struct ip_mreq
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 30

void error_handling(char *message);

int main(void) {
	WSADATA			wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error");
	}

	SOCKET recv_sock = socket(PF_INET, SOCK_DGRAM, 0); // UDP

	// 1. 서버 주소 binding
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // IPv4
	addr.sin_port = htons(9000); // Big endian (Port 9000)
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(recv_sock, (SOCKADDR*)&addr, sizeof(addr));
	
	// 2. 멀티캐스트 주소 수신하도록 설정하기: 소켓 옵션 사용
	struct ip_mreq join_addr;
	join_addr.im_multiaddr.s_addr = inet_addr("224.1.1.2"); // Class D Multicast Address
	join_addr.im_interface.s_addr = htonl(INADDR_ANY); // IP 주소 알아서 설정

	setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr));
	
	// 3. 반복적으로 수신하기 -> 수신 여부 확인
	char buf[BUF_SIZE];

	while (1) {
		int str_len = recvfrom(recv_sock, buf, BUF_SIZE-1, 0, NULL, 0);

		if (str_len < 0) { // strLen > 0, strLen < 0: 오류 상황
			break;
		}
		
		buf[str_len] = 0; // 스트링의 끝 표시
		printf("%s", buf);
	}

	closesocket(recv_sock);
	WSACleanup();
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}