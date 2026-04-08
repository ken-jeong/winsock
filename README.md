# Socket Programming Examples

![Bootcamp](https://img.shields.io/badge/Bootcamp-Socket%20Programming-0A66C2?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Winsock2](https://img.shields.io/badge/API-Winsock2-2E7D32?style=for-the-badge)
![TCP/IP](https://img.shields.io/badge/Network-TCP%2FIP-FF6F00?style=for-the-badge)
![UDP](https://img.shields.io/badge/Protocol-UDP-6A1B9A?style=for-the-badge)
![Select](https://img.shields.io/badge/I%2FO-select-455A64?style=for-the-badge)
![Threading](https://img.shields.io/badge/Topic-Threading-D32F2F?style=for-the-badge)
![Build](https://img.shields.io/badge/Build-GCC%20%2B%20ws2__32-795548?style=for-the-badge)

Windows Winsock 기반 C 소켓 프로그래밍 예제 모음입니다. 기본 TCP/UDP 통신부터 주소 변환, 소켓 옵션, `select` 기반 I/O 멀티플렉싱, 멀티캐스트/브로드캐스트, 스레드, 동기화 객체, WSA 이벤트, Overlapped I/O 예제를 다룹니다.

## 요구 환경

- Windows
- MinGW-w64 또는 GCC 호환 C 컴파일러
- Winsock2 라이브러리(`ws2_32`)

모든 네트워크 예제는 Winsock을 사용하므로 빌드할 때 `-lws2_32` 옵션을 함께 지정해야 합니다.

## 구성

### `midterm/`

기본 소켓 API와 TCP/UDP 통신 예제입니다.

| 파일 | 내용 |
| --- | --- |
| `ch01_tcp-server.c` | 기본 TCP 서버 |
| `ch01_udp-server.c` | 기본 UDP 서버 |
| `ch02_tcp-server.c`, `ch02_tcp-client.c` | TCP 서버/클라이언트 |
| `ch02_udp-server.c`, `ch02_udp-client.c` | UDP 서버/클라이언트 |
| `ch03_server.c` | 주소 구조체, `WSAStringToAddress`, `WSAAddressToString` |
| `ch04_tcp-server.c`, `ch04_tcp-client.c` | 반복 송수신 TCP 예제 |
| `ch05_tcp-cal-server.c`, `ch05_tcp-cal-client.c` | TCP 계산기 서버/클라이언트 |
| `ch06_bound-server.c`, `ch06_bound-client.c` | UDP `bind` 동작 확인 |
| `ch07_shutdown-server.c`, `ch07_shutdown-client.c` | TCP `shutdown` 예제 |
| `ch08_gethostbyname.c`, `ch08_gethostbyaddr.c` | 호스트 이름/IP 주소 조회 |
| `ch09_sockopt.c` | 소켓 옵션 조회 |

### 루트 예제

| 파일 | 내용 |
| --- | --- |
| `ch12_select-server.c`, `ch12_select-client.c` | `select` 기반 I/O 멀티플렉싱 서버/클라이언트 |
| `ch14-1_multicast-sendto.c`, `ch14-1_multicast-recvfrom.c` | 멀티캐스트 송수신 |
| `ch14-2_broadcast-sendto.c`, `ch14-2_broadcast-recvfrom.c` | 브로드캐스트 송수신 |
| `ch19_WaitForSingleObject.c` | `_beginthreadex`와 `WaitForSingleObject` |
| `ch19_beginthreadex-server.c`, `ch19_beginthreadex-client.c` | 스레드 기반 TCP 서버/클라이언트 |
| `ch20-1_sync-critical-section.c` | Critical Section 동기화 |
| `ch20-2_sync-mutex.c` | Mutex 동기화 |
| `ch20-3_sync-semaphore.c` | Semaphore 동기화 |
| `ch20-4_sync-event.c` | Event 동기화 |
| `ch21_WSAEvent-server.c`, `ch21_WSAEvent-client.c` | `WSAEventSelect`, `WSAWaitForMultipleEvents` 기반 이벤트 서버/클라이언트 |
| `ch22-1_WSARecv.c`, `ch22-1_WSASend.c` | 이벤트 객체를 사용하는 Overlapped 송수신 |
| `ch22-2_ CompletionRoutineRecv.c` | Completion routine 기반 Overlapped 수신 |
| `ch14_data.txt` | 멀티캐스트/브로드캐스트 송신 예제 입력 파일 |

## 빌드

각 소스 파일 첫 줄에 해당 파일을 빌드하는 명령이 주석으로 적혀 있습니다.

```bash
gcc ch12_select-server.c -o ch12_select-server.exe -lws2_32
gcc ch12_select-client.c -o ch12_select-client.exe -lws2_32
gcc ch21_WSAEvent-server.c -o ch21_WSAEvent-server.exe -lws2_32
gcc ch22-1_WSARecv.c -o ch22-1_WSARecv.exe -lws2_32
gcc ch22-1_WSASend.c -o ch22-1_WSASend.exe -lws2_32
gcc "ch22-2_ CompletionRoutineRecv.c" -o "ch22-2_ CompletionRoutineRecv.exe" -lws2_32
```

`midterm/` 아래 파일은 출력 파일도 같은 디렉터리에 생성하도록 빌드합니다.

```bash
gcc midterm/ch02_tcp-server.c -o midterm/ch02_tcp-server.exe -lws2_32
gcc midterm/ch02_tcp-client.c -o midterm/ch02_tcp-client.exe -lws2_32
```

## 실행

서버와 클라이언트가 쌍으로 있는 예제는 서버를 먼저 실행한 뒤 다른 터미널에서 클라이언트를 실행합니다.

```bash
./ch12_select-server.exe
./ch12_select-client.exe
```

대부분의 네트워크 예제는 `127.0.0.1` 또는 `INADDR_ANY`와 `9000` 포트를 사용합니다. 같은 포트를 사용하는 프로그램이 이미 실행 중이면 해당 소스의 포트 번호를 변경한 뒤 다시 빌드하세요.

멀티캐스트/브로드캐스트 송신 예제는 실행 위치에 `ch14_data.txt`가 있어야 합니다.

```bash
./ch14-1_multicast-recvfrom.exe
./ch14-1_multicast-sendto.exe
```

Overlapped I/O 송수신 예제도 한쪽을 먼저 대기시킨 뒤 다른 쪽을 실행합니다.

```bash
./ch22-1_WSARecv.exe
./ch22-1_WSASend.exe
```

`ch22-2_ CompletionRoutineRecv.c`는 completion routine 호출을 위해 alertable wait 상태가 필요하므로 코드의 `WSAWaitForMultipleEvents(..., TRUE)` 흐름을 함께 확인하세요.

## 참고

- 이 저장소는 학습용 단일 파일 예제로 구성되어 있으며 별도의 Makefile이나 프로젝트 파일은 포함하지 않습니다.
- Windows 전용 헤더와 API(`winsock2.h`, `windows.h`, `_beginthreadex`)를 사용하므로 Linux/macOS에서는 그대로 빌드되지 않습니다.
- 실행 파일(`*.exe`)과 임시 산출물은 저장소에 포함하지 않는 것을 권장합니다.
