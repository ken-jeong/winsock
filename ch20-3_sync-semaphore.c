// gcc ch20-3_sync-semaphore.c -o ch20-3_sync-semaphore.exe -lws2_32

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <process.h>

#define LOOP_COUNT 5

unsigned WINAPI input_thread(void* arg);
unsigned WINAPI sum_thread(void* arg);
unsigned WINAPI product_thread(void* arg);

static int shared_number = 1; // 공유 데이터

static HANDLE h_input_ready_sem;
static HANDLE h_sum_ready_sem;
static HANDLE h_product_ready_sem;

int main(void) {
    // 세마포어 생성하기
    // input_thread가 첫 루프에서 대기 없이 바로 실행
    h_input_ready_sem   = CreateSemaphore(NULL, 2, 2, NULL); // 초기값 2, 최대값 2
    h_sum_ready_sem     = CreateSemaphore(NULL, 0, 1, NULL); // 초기값 0, 최대값 1
    h_product_ready_sem = CreateSemaphore(NULL, 0, 1, NULL); // 초기값 0, 최대값 1

    // 스레드 생성
    HANDLE hThread_input   = (HANDLE)_beginthreadex(NULL, 0, input_thread,   NULL, 0, NULL);
    HANDLE hThread_sum     = (HANDLE)_beginthreadex(NULL, 0, sum_thread,     NULL, 0, NULL);
    HANDLE hThread_product = (HANDLE)_beginthreadex(NULL, 0, product_thread, NULL, 0, NULL);

    // 스레드 종료 대기하기
    WaitForSingleObject(hThread_input,   INFINITE);
    WaitForSingleObject(hThread_sum,     INFINITE);
    WaitForSingleObject(hThread_product, INFINITE);

    // 스레드 핸들 닫기
    CloseHandle(hThread_input);
    CloseHandle(hThread_sum);
    CloseHandle(hThread_product);

    // 세마포어 핸들 닫기
    CloseHandle(h_input_ready_sem);
    CloseHandle(h_sum_ready_sem);
    CloseHandle(h_product_ready_sem);
    return 0;
}

unsigned WINAPI input_thread(void* arg) {
    for (int i = 0; i < LOOP_COUNT; i++) {
        // 1. 숫자 입력 받기
        printf("input_thread> Input number: "); // 입력 프롬프트 출력
        fflush(stdout); // 버퍼를 비워 입력을 대기하기 전에 화면에 바로 출력되도록 합니다.

        // 2. 다른 누적 스레드들이 누적 완료를 대기
        // 두 개의 누적 스레드(sum_thread, product_thread)가 모두 작업을 마칠 때까지 대기합니다.
        WaitForSingleObject(h_input_ready_sem, INFINITE);
        WaitForSingleObject(h_input_ready_sem, INFINITE);

        // 3. 숫자를 입력하기
        scanf("%d", &shared_number);

        // 4. 다른 스레드들이 입력된 숫자를 처리하도록 유도하기
        ReleaseSemaphore(h_sum_ready_sem,     1, NULL);
        ReleaseSemaphore(h_product_ready_sem, 1, NULL);
    }
    return 0;	
}

unsigned WINAPI sum_thread(void* arg) {
    int sum = 0;

    for (int i = 0; i < LOOP_COUNT; i++) {
        WaitForSingleObject(h_sum_ready_sem, INFINITE); // input_thread가 숫자를 입력하기를 대기

        sum += shared_number; // 숫자 누적하기

        ReleaseSemaphore(h_input_ready_sem, 1, NULL); // input_thread가 다른 숫자를 입력하도록 유도
        printf("sum_thread> result: %d\n", sum); // 누적된 숫자 출력
    }	
    return 0;
}

unsigned WINAPI product_thread(void* arg) {
    int product = 1;

    for (int i = 0; i < LOOP_COUNT; i++) {
        WaitForSingleObject(h_product_ready_sem, INFINITE); // input_thread가 숫자를 입력하기를 대기

        product *= shared_number; // 숫자 누적하기

        ReleaseSemaphore(h_input_ready_sem, 1, NULL); // input_thread가 다른 숫자를 입력하도록 유도
        printf("product_thread> result: %d\n", product); // 누적된 숫자 출력
    }
    return 0;
}

/*
        sum_thread> result: 1
    product_thread> result: 1
        sum_thread> result: 3
    product_thread> result: 2
        sum_thread> result: 6
    product_thread> result: 6
        sum_thread> result: 10
    product_thread> result: 24
        sum_thread> result: 15
    product_thread> result: 120
*/