// gcc ch20-4_syncThread-Event.c -o ch20-4_syncThread-Event.exe -lws2_32

/*
    입력 ABCAa이면
    Number of 'A': 2
    Number of others: 3
*/

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define STR_LEN 100

unsigned WINAPI number_of_a(void *arg);
unsigned WINAPI number_of_others(void *arg);

static HANDLE hEvent;
static char input_string[STR_LEN];

int main(int argc, char *argv[]) {
    // 1. 이벤트 객체 생성하기 (manual reset, 초기값 FALSE)
    // CreateEvent(보안 속성, bManualReset, bInitialState, 이름)
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (hEvent == NULL) {
        printf("Failed to create event object.\n");
        return -1;
    }

    // 스레드 생성
    HANDLE h_thread1 = (HANDLE)_beginthreadex(NULL, 0, number_of_a,      NULL, 0, NULL);
    HANDLE h_thread2 = (HANDLE)_beginthreadex(NULL, 0, number_of_others, NULL, 0, NULL);

    if (h_thread1 == NULL || h_thread2 == NULL) {
        printf("Failed to create threads.\n");
        return -1;
    }

    // 2. 사용자가 스트링 입력하기
    printf("Enter a string: ");
    if (fgets(input_string, STR_LEN, stdin) != NULL) {
        // 입력받은 문자열 끝의 개행 문자(\n) 제거
        input_string[strcspn(input_string, "\n")] = '\0';
    }

    // 3. 이벤트 객체 대상으로 signaled 상태로 설정하여 스레드 진행
    SetEvent(hEvent);

    // 커널 오브젝트 signaled 상태 대기
    WaitForSingleObject(h_thread1, INFINITE);
    WaitForSingleObject(h_thread2, INFINITE);

    // 이벤트 상태 초기화 및 핸들 반환
    ResetEvent(hEvent);
    CloseHandle(hEvent);

    // 생성한 스레드 핸들 반환 (리소스 누수 방지)
    CloseHandle(h_thread1);
    CloseHandle(h_thread2);

    return 0;
}

unsigned WINAPI number_of_a(void *arg) {
    int cnt = 0;

    // 메인 스레드에서 SetEvent를 호출할 때까지 블로킹
    WaitForSingleObject(hEvent, INFINITE); // 커널 오브젝트 signaled 상태 대기

    for (int i = 0; input_string[i] != '\0'; i++) {
        if (input_string[i] == 'A') { // 'A' 글자 수 계수
            cnt++;
        }
    }

    printf("Number of 'A': %d\n", cnt);
    return 0;
}

unsigned WINAPI number_of_others(void *arg) {
    int cnt = 0;

    // 메인 스레드에서 SetEvent를 호출할 때까지 블로킹
    WaitForSingleObject(hEvent, INFINITE); // 커널 오브젝트 signaled 상태 대기

    for (int i = 0; input_string[i] != '\0'; i++) {
        if (input_string[i] != 'A') { // 'A' 외의 글자 수 계수
            cnt++;
        }
    }

    printf("Number of others: %d\n", cnt);
    return 0;
}
