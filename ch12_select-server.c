// gcc ch12_select-server.c -o ch12_select-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024

static void handle_new_connection(SOCKET serv_sock, fd_set *reads);
static void handle_client_data(SOCKET sock, fd_set *reads, char *buf);
void error_handling(const char *message);

int main(void) {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error!");
	}

	SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family		= AF_INET;
	serv_adr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	serv_adr.sin_port		= htons(9000);

	if (bind(serv_sock, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR) {
		error_handling("bind() error!");
	}

	if (listen(serv_sock, 5) == SOCKET_ERROR) {
		error_handling("listen() error!");
	}
	
	/*
		1단계: 서버소켓 생성 및 관찰 대상 등록
	*/
	fd_set reads; // 소켓 관리를 위한 fd_set 구조체 변수
	// reads 셋 초기화 및 서버 소켓 추가
	FD_ZERO(&reads); // reads 셋을 비움 (초기화)
	FD_SET(serv_sock, &reads); // serv_sock을 감시 대상으로 등록 (클라이언트의 연결 요청 감시)

	char buf[BUF_SIZE];

	while (1) { // 서버를 계속 실행 — select → 이벤트 처리를 반복
		TIMEVAL timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		/*
			2단계: select 호출 루프
		*/
		// select 호출 시 원본 fd_set이 변경되므로 복사본(cpy_reads)을 생성하여 전달
		fd_set cpy_reads = reads;
		// select: 감시 대상 소켓들에 I/O 이벤트가 발생했는지 확인 (I/O 멀티플렉싱)
		int fd_num = select(
			0,			// 검사 대상 파일 디스크립터의 수
			&cpy_reads,	// 수신 데이터 존재 여부를 관찰할 디스크립터 집합
			0,			// 블로킹 없는 전송 가능 여부를 관찰할 디스크립터 집합
			0,			// 예외상황 발생 여부를 관찰할 디스크립터 집합
			&timeout	// 무한정 블로킹을 막기 위한 타임아웃 설정
		);
		
		if (fd_num == SOCKET_ERROR) { // select 함수 오류 발생 시 종료
			break;
		}

		if (fd_num == 0) { // 타임아웃 발생 시 다시 루프의 처음으로 이동
			continue;
		}
		
		/*
			3단계: 이벤트 처리
		*/
		// 이벤트가 발생한 소켓이 있는지 fd_set의 등록된 개수만큼 반복하며 확인
		// 발생한 이벤트가 연결 요청인지, 데이터 수신인지 구분하여 처리
		for (int i = 0; i < reads.fd_count; i++) {
			if (!FD_ISSET(reads.fd_array[i], &cpy_reads)) { // 해당 소켓에 이벤트가 발생했는지 확인
				continue;
			}

			// 서버 소켓(serv_sock)에 이벤트 발생: 새로운 클라이언트의 연결 요청
			if (reads.fd_array[i] == serv_sock) {
				handle_new_connection(serv_sock, &reads);
			// 클라이언트 소켓에 이벤트 발생: 데이터 수신 혹은 연결 종료
			} else {
				handle_client_data(reads.fd_array[i], &reads, buf);
			}
		}
	}
	closesocket(serv_sock);
	WSACleanup();
	return 0;
}

// 서버 소켓(serv_sock)에 이벤트 발생: 새로운 클라이언트의 연결 요청 처리
static void handle_new_connection(SOCKET serv_sock, fd_set *reads) {
	SOCKADDR_IN clnt_adr;
	int adr_sz = sizeof(clnt_adr);
	SOCKET clnt_sock = accept(serv_sock, (SOCKADDR*)&clnt_adr, &adr_sz);

	// 새로 연결된 클라이언트 소켓을 감시 대상(reads)에 추가
	FD_SET(clnt_sock, reads);

	// 포트 번호 정상 출력을 위해 ntohs() 적용
	printf("Connected client: Port: %d, IP: %s \n",
		ntohs(clnt_adr.sin_port), inet_ntoa(clnt_adr.sin_addr)
	);
}

// 클라이언트 소켓에 이벤트 발생: 데이터 수신 혹은 연결 종료 처리
static void handle_client_data(SOCKET sock, fd_set *reads, char *buf) {
	int str_len = recv(sock, buf, BUF_SIZE - 1, 0);

	// 수신한 데이터 길이가 0 이하인 경우: 클라이언트의 연결 종료 요청
	if (str_len <= 0) {
		closesocket(sock);
		printf("Closed client: SOCKET %d, str_len: %d \n", (int)sock, str_len);

		// 연결이 종료된 소켓을 감시 대상(reads 셋)에서 제거
		FD_CLR(sock, reads);
		return;
	}

	// 정상적으로 데이터를 수신한 경우
	// 메시지를 보낸 클라이언트의 주소 정보 가져오기
	SOCKADDR_IN client_addr;
	int addr_len = sizeof(client_addr);
	getpeername(sock, (SOCKADDR*)&client_addr, &addr_len);

	// 문자열 끝에 NULL 문자를 추가하여 문자열 완성
	buf[str_len] = '\0';
	printf("(Port: %d, IP: %s), Msg: %s \n",
		ntohs(client_addr.sin_port), inet_ntoa(client_addr.sin_addr), buf
	);

	// 수신한 데이터를 클라이언트에게 그대로 재전송 (Echo 기능)
	send(sock, buf, str_len, 0);
}

/*
	select() 서버 흐름:
	연결 요청이면 Connected client,
	데이터 수신이면 메시지 출력 후 echo,
	연결 종료면 Closed client
*/

void error_handling(const char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}