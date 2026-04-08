// gcc ch20-1_syncThread-CS.c -o ch20-1_syncThread-CS.exe -lws2_32

#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <time.h>

#define NUM_THREAD 50
#define LOOP_CNT   100000

unsigned WINAPI increase_thread(void *arg);
unsigned WINAPI decrease_thread(void *arg);

long long shared_number = 0; // 전역 변수

CRITICAL_SECTION shared_number_cs; // 동기화 객체 선언

int main(void) {
    int i;
    HANDLE hThreads[NUM_THREAD];

    InitializeCriticalSection(&shared_number_cs); // 동기화 객체 초기화
    clock_t start = clock();

    // 스레드 생성
    for (i = 0; i < NUM_THREAD; i++) {
        if (i % 2 == 0) {
            hThreads[i] = (HANDLE)_beginthreadex( // +-< _beginthreadex() >---+
                NULL,            0,               // | winsock: NULL, 0       |
                increase_thread, NULL,            // | thread_func, parameter |
                0,               NULL             // | initflag,    thread_id |
            );                                    // +------------------------+
        } else {
            hThreads[i] = (HANDLE)_beginthreadex( // +-< _beginthreadex() >---+
                NULL,            0,               // | winsock: NULL, 0       |
                decrease_thread, NULL,            // | thread_func, parameter |
                0,               NULL             // | initflag,    thread_id |
            );                                    // +------------------------+
        }
    }

    // 커널 오브젝트 signaled 상태 대기
    WaitForMultipleObjects(
        NUM_THREAD, hThreads, // 오브젝트 수, 오브젝트 배열
        TRUE,       INFINITE  // 전부, timeout
    );

    clock_t end = clock();
    DeleteCriticalSection(&shared_number_cs); // 동기화 객체 삭제

    // 최종 전역 변수 값 출력하기
    printf("shared_number = %lld\n", shared_number); // ** shared_number = 0 **
    printf("> Elapsed time = %lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    return 0;
}

unsigned WINAPI increase_thread(void *arg) {
    EnterCriticalSection(&shared_number_cs); // 동기화 객체 획득

    for (int i = 0; i < LOOP_CNT; i++) {
        shared_number++;
        // MOV R1, num
        // ADD R1, 1
        // STO R1, num
    }

    LeaveCriticalSection(&shared_number_cs); // 동기화 객체 반납
    return 0;
}

unsigned WINAPI decrease_thread(void *arg) {
    EnterCriticalSection(&shared_number_cs); // 동기화 객체 획득

    for (int i = 0; i < LOOP_CNT; i++) {
        shared_number--;
        // MOV R2, num
        // SUB R2, 1
        // STO R2, num
    }

    LeaveCriticalSection(&shared_number_cs); // 동기화 객체 반납
    return 0;
}
