// gcc ch23_IOCP-server.c -o ch23_IOCP-server.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_LEN 100
#define READ     3
#define	WRITE    5

typedef struct { // 클라이언트 소켓 정보 저장
	SOCKET      client_sock;
	SOCKADDR_IN client_addr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct { // 입출력(I/O) 정보 저장
	OVERLAPPED overlapped;
	WSABUF     wsabuf;
	char       buf[BUF_LEN];
	int        rwMode;
} PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void error_handling(char *msg);

int main(void) {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	HANDLE comp_port = CreateIoCompletionPort( // < CreateIoCompletionPort() >
		INVALID_HANDLE_VALUE, NULL, 0,         // INVALID_HANDLE_VALUE, NULL, 0
		0                                      // 시스템 CPU 수만큼
	);

	// 2. 시스템 정보 확인 및 쓰레드 생성하기
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	for (int i = 0; i < (int)sysInfo.dwNumberOfProcessors; i++) {
		_beginthreadex(
			NULL, 0, // winsock: NULL, 0
			EchoThreadMain, (LPVOID)comp_port,
			0, NULL  // initflag, 스레드 ID
		);
	}
	
	SOCKET server_sock = WSASocket(        // < WSASocket() >
		AF_INET, SOCK_STREAM, IPPROTO_TCP, // 주소, 소켓, 프로토콜
		NULL, 0,                           // 보통 NULL, 0
		WSA_FLAG_OVERLAPPED                // 플래그
	);

	SOCKADDR_IN server_addr;
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	server_addr.sin_port		= htons(9000);

	bind(server_sock, (SOCKADDR*)&server_addr, sizeof(server_addr));
	
	listen(server_sock, 5);
	
	while (1) {
		SOCKADDR_IN client_addr;		
		int         sizeof_addr = sizeof(client_addr);
		SOCKET      client_sock = accept(server_sock, (SOCKADDR*)&client_addr, &sizeof_addr);
		
		printf("Server> Client connection request: IP %s (len:%d)\n",
			inet_ntoa(client_addr.sin_addr),
			sizeof_addr
		);

		// 클라이언트 핸들 정보를 위한 메모리 할당하기
		LPPER_HANDLE_DATA comp_key = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		comp_key->client_sock      = client_sock;
		memcpy(&(comp_key->client_addr), &client_addr, sizeof(client_addr));

		// ================================================== //
		// 3. CreateIoCompletionPort(): IOCP에 소켓 등록하기
		// ================================================== //
		CreateIoCompletionPort(
			(HANDLE)client_sock, comp_port, (ULONG_PTR)comp_key,
			0 // 시스템 CPU 수만큼
		);

		// 4. 입출력 데이터 및 overlapped 구조체 설정하기
		LPPER_IO_DATA ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsabuf.len = BUF_LEN;
		ioInfo->wsabuf.buf = ioInfo->buf;
		ioInfo->rwMode     = READ;

		DWORD bytes_len, flags = 0;
		WSARecv(                               // < WSARecv() >
			client_sock, &(ioInfo->wsabuf), 1, // 소켓, 버퍼, 버퍼 수
			&bytes_len, &flags,                // 바이트 수, 플래그
			&(ioInfo->overlapped), NULL        // overlapped, completion_routine
		);
	}
	return 0;
}

// 5. 워커 쓰레드 메인 함수
DWORD WINAPI EchoThreadMain(LPVOID pComPort) {
	HANDLE            comp_port = (HANDLE)pComPort;
	DWORD             bytes_len;
	LPPER_HANDLE_DATA comp_key;
	LPPER_IO_DATA     io_data;
	LPPER_IO_DATA     next_io_data;
	SOCKET            client_sock;
	DWORD             flags = 0;

	while (1) {
		// ================================================== //
		// 1. GetQueuedCompletionStatus()
		// IOCP로부터 완료된 입출력 완료 상태 얻기
		// ================================================== //
		GetQueuedCompletionStatus(
			comp_port,
			&bytes_len, (PULONG_PTR)&comp_key, (LPOVERLAPPED*)&io_data,
			INFINITE
		);
		
		client_sock = comp_key->client_sock;

		if (io_data->rwMode == READ) {
			printf("Thread> message received!\n");
			if (bytes_len == 0) { // EOF 수신 시
				closesocket(client_sock);
				free(comp_key);
				free(io_data);
				continue;
			}

			// 2. 수신한 데이터를 에코 백(echo back)
			memset(&(io_data->overlapped), 0, sizeof(OVERLAPPED));
			io_data->wsabuf.len = bytes_len; // 수신한 바이트 수만큼 설정
			io_data->rwMode     = WRITE;

			printf("Thread> call send().\n");

			WSASend(                                // < WSASend() >
				client_sock, &(io_data->wsabuf), 1, // 소켓, 버퍼, 버퍼 수
				NULL, 0,                            // 바이트 수, 플래그
				&(io_data->overlapped), NULL        // overlapped, completion_routine
			);

			// 3. 추가적인 데이터 수신 대기 설정
			next_io_data = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(next_io_data->overlapped), 0, sizeof(OVERLAPPED));
			next_io_data->wsabuf.len = BUF_LEN;
			next_io_data->wsabuf.buf = next_io_data->buf;
			next_io_data->rwMode     = READ;

			printf("Thread> call recv()\n");

			WSARecv(                                     // < WSARecv() >
				client_sock, &(next_io_data->wsabuf), 1, // 소켓, 버퍼, 버퍼 수
				NULL, &flags,                            // 바이트 수, 플래그
				&(next_io_data->overlapped), NULL        // overlapped, completion_routine
			);
		} else {
			printf("Thread> message sent!\n");
			free(io_data);
		}
	}
	return 0;
}

void error_handling(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
