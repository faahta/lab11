#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#define MAX_TO_PRODUCE 2

DWORD P, C, N, T;
typedef struct buffer {
	DWORD *buffer;
	DWORD in, out, capacity;
}Buffer_t;
Buffer_t buf;
HANDLE semEmpty, semFull, meP, meC;
DWORD countP, countC;

DWORD WINAPI producerThreads(LPVOID);
DWORD WINAPI consumerThreads(LPVOID);
VOID send(DWORD);
DWORD receive(VOID);

int _tmain(int argc, LPTSTR argv[]) {
	P = _ttoi(argv[1]);
	C = _ttoi(argv[2]);
	N = _ttoi(argv[3]);
	T = _ttoi(argv[4]);

	buf.buffer = (DWORD*)malloc(N * sizeof(DWORD));
	buf.in = buf.out = 0;
	buf.capacity = N;

	semEmpty = CreateSemaphore(NULL, N, N, NULL);
	semFull = CreateSemaphore(NULL, 0, N, NULL);
	meP = CreateMutex(NULL, FALSE, NULL);
	meC = CreateMutex(NULL, FALSE, NULL);

	HANDLE* hPthreads, * hCThreads;
	hPthreads = (HANDLE*)malloc(P * sizeof(HANDLE));
	hCThreads = (HANDLE*)malloc(C * sizeof(HANDLE));
	DWORD i;
	DWORD* pi;
	for (i = 0; i < P; i++) {
		pi = (DWORD*)malloc(sizeof(DWORD));
		*pi = i;
		hPthreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)producerThreads, pi, 0, NULL);
	}

	for (i = 0; i < C; i++) {
		pi = (DWORD*)malloc(sizeof(DWORD));
		*pi = i;
		hPthreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)consumerThreads, pi, 0, NULL);
	}
	WaitForMultipleObjects(P, hPthreads, TRUE, INFINITE);
	
	Sleep(1000);
	free(buf.buffer);
	free(hPthreads);
	

	WaitForMultipleObjects(C, hCThreads, TRUE, INFINITE);
	free(hCThreads);
	return 0;
}


DWORD WINAPI producerThreads(LPVOID lpParam) {
	DWORD* id = (DWORD*)lpParam;
	float r; 
	DWORD wt, data;

	srand(*id);
	countP = 0;
	while (countP < MAX_TO_PRODUCE) {
		 r = ((float)(rand())) / ((float)(RAND_MAX));
		 wt = (DWORD)(r * T);	
		 Sleep(wt * 1000);
		 data = rand();
		 send(data);

		_tprintf(_T("producer %d : produced: %d\n"), *id, data);
		countP++;
	}
	_tprintf(_T("PRODUCER %d EXITING...\n"), *id);
	ExitThread(0);

}
DWORD WINAPI consumerThreads(LPVOID lpParam) {
	DWORD* id = (DWORD*)lpParam;
	float r;
	DWORD wt, data;

	srand(*id);
	while (1) {
		r = ((float)(rand())) / ((float)(RAND_MAX));
		wt = (DWORD)(r * T);
		Sleep(wt * 1000);
		if (buf.buffer == NULL) {
			_tprintf(_T("consumer %d exiting...\n"),*id);
			break;
		}
			
		DWORD data = receive();
		_tprintf(_T("consumer %d: received data: %d\n"), *id, data);
	}
	ExitThread(0);
}

VOID send(DWORD data) {
	WaitForSingleObject(semEmpty, INFINITE);
	WaitForSingleObject(meP, INFINITE);
	buf.buffer[buf.in] = data;
	buf.in = (buf.in + 1) % buf.capacity;
	ReleaseMutex(meP);
	ReleaseSemaphore(semFull, 1, NULL);
}
DWORD receive(VOID) {
	DWORD data;
	WaitForSingleObject(semFull, INFINITE);
	WaitForSingleObject(meC, INFINITE);
	data = buf.buffer[buf.out];
	buf.out = (buf.out + 1) % buf.capacity;
	ReleaseMutex(meC);
	ReleaseSemaphore(semEmpty, 1, NULL);
	return data;
}