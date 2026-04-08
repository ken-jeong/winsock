// gcc ch19_WaitForSingleObject.c -o ch19_WaitForSingleObject.exe -lws2_32

#include <stdio.h>
#include <process.h>
#include <windows.h>

unsigned WINAPI count_thread(void* arg);

int main() {
    int      parameter = 10; /** 10번 **/
    unsigned thread_id;

    HANDLE h_thread = (HANDLE)_beginthreadex( // +-< _beginthreadex() >---+
        NULL,         0,                      // | winsock: NULL, 0       |
        count_thread, (void*)&parameter,      // | thread_func, parameter |
        0,            &thread_id              // | initflag,    thread_id |
    );                                        // +------------------------+

    if (h_thread == 0) { // 생성 실패 시
        printf("<ERROR> _beginthreadex() fail.\n");
        return -1;
    }

    // Sleep(6000); // 6초 sleep
    
    int result = WaitForSingleObject(h_thread, INFINITE); // < WaitForSingleObject() >

    printf("main> wait result: %s\n", (result==WAIT_OBJECT_0)?"signaled":"time-out");
    printf("main> end of main\n");
    return 0;
}

unsigned WINAPI count_thread(void *arg) {
    int count = *((int*)arg);
    printf("count_thread> count: %d\n", count);

    for (int i = 0; i < count; i++) {
        Sleep(1000);
        printf("count_thread> running thread.\n");
    }
    return 0;
}
