// gcc ch19_WaitForSingleObject.c -o ch19_WaitForSingleObject.exe -lws2_32

#include <stdio.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI count_thread(void* arg);

int main() {
	int param = 10;
	unsigned thread_id;

	HANDLE h_thread = (HANDLE)_beginthreadex(
		NULL, 			// 보안 속성 (NULL: 기본 보안 속성 사용)
		0, 				// 스택 크기 (0: 기본 스택 크기 사용)
		count_thread, 	// 스레드 함수
		(void*)&param, 	// 스레드 함수에 전달할 인자
		0, 				// 생성 + 실행 (0: 생성 즉시 스레드 실행)
		&thread_id 		// 스레드 ID
	);

	if (h_thread == 0) { // 생성 실패 시, 
		printf("<ERROR> _beginthreadex() fail.\n");
		return -1;
	}

	// Sleep(6000); // 6초 sleep
	int ret = WaitForSingleObject(h_thread, INFINITE);
	printf("main> wait result: %s\n", (ret==WAIT_OBJECT_0)?"signaled":"time-out");

	printf("main> end of main\n");
	return 0;
}

unsigned WINAPI count_thread(void *arg) {
	int count = *((int*)arg);
	printf("thread> count: %d\n", count);

	for (int i = 0; i < count; i++) {
		Sleep(1000);
		printf("thread> running thread.\n"); // ** 10번 출력 **
	}

	return 0;
}