// gcc ch21_WSAEvent-server.c -o ch21_WSAEvent-server.exe -lws2_32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 100

void compress_sockets(SOCKET socket_array[], int deleted_index, int total_count);
void compress_events(WSAEVENT event_array[], int deleted_index, int total_count);
void error_handling(char *error_message);

int main() {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		error_handling("WSAStartup() error!");
	}

	SOCKET server_socket = socket(PF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN server_addr;
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(9000);

	if (bind(server_socket, (SOCKADDR*) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		error_handling("bind() error");
	}

	if (listen(server_socket, 5) == SOCKET_ERROR) {
		error_handling("listen() error");
	}
	
	/*
		1. CreateEvent()
		이벤트 객체 생성
	*/
	WSAEVENT new_event = WSACreateEvent();

	/*
		2. WSAEventSelect()
		소켓에 이벤트 객체를 연결하고, 감지할 네트워크 이벤트를 등록한다.
	*/
	if (WSAEventSelect(server_socket, new_event, FD_ACCEPT) == SOCKET_ERROR) {
		error_handling("WSAEventSelect() error");
	}
	
	// 소켓과 이벤트 객체를 배열에 저장
	SOCKET   socket_array[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT  event_array[WSA_MAXIMUM_WAIT_EVENTS];
	int socket_count = 0;

	socket_array[socket_count] = server_socket;
	event_array[socket_count++] = new_event;
	char message_buffer[BUF_SIZE];
	while(1) {
		/*
			3. WSAWaitForMultipleEvents()
			등록된 이벤트 객체들 중 하나 이상이 signaled 상태가 될 때까지 대기한다.
		*/
		int wait_result = WSAWaitForMultipleEvents(
			socket_count, event_array, FALSE, WSA_INFINITE, FALSE
		);
		
		int first_signaled_index = wait_result - WSA_WAIT_EVENT_0; // Signalled 상태 이벤트 객체 확인

		for (int i = first_signaled_index; i < socket_count; i++) { // Signal이 발생한 이벤트에 대해서 이벤트 확인 및 처리
			int wait_single_result = WSAWaitForMultipleEvents(
				1, &event_array[i], TRUE, 0, FALSE
			);

			if ((wait_single_result == WSA_WAIT_FAILED || wait_single_result == WSA_WAIT_TIMEOUT)) {
				continue;
			} else {
				/*
					4. WSAEnumNetworkEvents()
					signaled된 이벤트 객체에서 실제로 발생한 네트워크 이벤트와 오류 코드를 확인한다.
				*/
				WSANETWORKEVENTS networkEvents; // 발생한 이벤트 구체화 때 사용하는 구조체 변수
				WSAEnumNetworkEvents(socket_array[i], event_array[i], &networkEvents);

				/*
					4.1. FD_ACCEPT 이벤트 처리
				*/
				if (networkEvents.lNetworkEvents & FD_ACCEPT) {
					if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
						puts("Accept Error");
						break;
					}

					SOCKADDR_IN client_addr;
					int sizeof_client_addr = sizeof(client_addr);
					SOCKET client_socket = accept(socket_array[i], (SOCKADDR*)&client_addr, &sizeof_client_addr);

					new_event = WSACreateEvent(); // 새로운 이벤트 생성
					WSAEventSelect(client_socket, new_event, FD_READ | FD_CLOSE); // 이벤트와 소켓 연결 및 등록

					socket_array[socket_count] = client_socket;
					event_array[socket_count++] = new_event;
					puts("connected new client...");
				}

				/*
					4.2. FD_READ 이벤트 처리
				*/
				if (networkEvents.lNetworkEvents & FD_READ) {
					if (networkEvents.iErrorCode[FD_READ_BIT] != 0) {
						puts("Read Error");
						break;
					}

					int received_bytes = recv(socket_array[i], message_buffer, sizeof(message_buffer), 0);
					message_buffer[received_bytes] = 0;
					printf("%s\n", message_buffer);
					send(socket_array[i], message_buffer, received_bytes, 0);
				}

				/*
					4.3. FD_CLOSE 이벤트 처리
				*/
				if (networkEvents.lNetworkEvents & FD_CLOSE) {
					if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
						puts("Close Error");
						break;
					}

					WSACloseEvent(event_array[i]); // 이벤트 소멸
					closesocket(socket_array[i]);  // 소켓 종료

					socket_count--;
					compress_sockets(socket_array, i, socket_count);
					compress_events(event_array, i, socket_count);
					i--;
				}
			}
		}
	}
	WSACleanup();
	return 0;
}

// 소켓 정보 배열 정리
void compress_sockets(SOCKET socket_array[], int deleted_index, int total_count) {
	for (int i = deleted_index; i < total_count; i++) {
		socket_array[i] = socket_array[i+1];
	}
}

// 이벤트 정보 배열 정리
void compress_events(WSAEVENT event_array[], int deleted_index, int total_count) {
	for(int i = deleted_index; i < total_count; i++) {
		event_array[i] = event_array[i+1];
	}
}

void error_handling(char *error_message) {	
	fputs(error_message, stderr);
	fputc('\n', stderr);
	exit(1);
}
