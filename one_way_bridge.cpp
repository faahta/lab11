/*Reader and Writer logical scheme modified to manipulate two sets of Readers.*/
#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct l2r_s {
	DWORD time_A;
	DWORD time_T;
	DWORD nThreads;
	HANDLE meL2R;
	DWORD nR;
}l2r_t;
typedef struct r2l_s {
	DWORD time_A;
	DWORD time_T;
	DWORD nThreads;
	HANDLE meR2L;
	DWORD nR;
}r2l_t;

HANDLE semW; /*bridge semaphore*/

l2r_t l2r;
r2l_t r2l;

DWORD WINAPI l2rThreads(LPVOID);
DWORD WINAPI r2lThreads(LPVOID);

DWORD WINAPI CARl2rThreads(LPVOID);
DWORD WINAPI CARr2lThreads(LPVOID);

int _tmain(int argc, LPTSTR argv[]) {
	
	l2r.time_A = _ttoi(argv[1]);
	l2r.time_T = _ttoi(argv[3]);
	l2r.nThreads = _ttoi(argv[5]);
	l2r.meL2R = CreateSemaphore(NULL, 1, l2r.nThreads, NULL);
	l2r.nR = 0;

	r2l.time_A = _ttoi(argv[2]);
	r2l.time_T = _ttoi(argv[4]);
	r2l.nThreads = _ttoi(argv[6]);
	r2l.meR2L = CreateSemaphore(NULL, 1, r2l.nThreads, NULL);
	r2l.nR = 0;

	semW = CreateSemaphore(NULL, 1, 1, NULL);

	HANDLE h_L2R_threads, h_R2L_threads;
	
	h_L2R_threads = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)l2rThreads, &l2r, 0, NULL);
	h_R2L_threads = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)r2lThreads, &r2l, 0, NULL);

	WaitForSingleObject(h_L2R_threads, INFINITE);
	WaitForSingleObject(h_R2L_threads, INFINITE);
	CloseHandle(h_L2R_threads);
	CloseHandle(h_R2L_threads);
	return 0;
}

DWORD WINAPI l2rThreads(LPVOID lpParam) {
	l2r_t* param = (l2r_t*)lpParam;
	HANDLE *hCars;
	hCars = (HANDLE*)malloc(param->nThreads * sizeof(HANDLE));
	INT i;
	for (i = 0; i < param->nThreads; i++){
		hCars[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CARl2rThreads, param, 0, NULL);
	}
	WaitForMultipleObjects(param->nThreads, hCars, TRUE, INFINITE);
	return (0);
}
DWORD WINAPI r2lThreads(LPVOID lpParam) {
	r2l_t* param = (r2l_t*)lpParam;
	HANDLE* hCars;
	hCars = (HANDLE*)malloc(param->nThreads * sizeof(HANDLE));
	INT i;
	for (i = 0; i < param->nThreads; i++) {
		hCars[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CARr2lThreads, param, 0, NULL);
	}
	WaitForMultipleObjects(param->nThreads, hCars, TRUE, INFINITE);
	return (0);
}

DWORD WINAPI CARl2rThreads(LPVOID lpParam) {
	l2r_t* param = (l2r_t*)lpParam;
	WaitForSingleObject(param->meL2R, INFINITE);
	param->nR++;
	if (param->nR == 1)
		WaitForSingleObject(semW, INFINITE);
	ReleaseSemaphore(param->meL2R, 1, NULL);
	//##CRITICAL REGION##
	_tprintf(_T("CAR L2R Crossing the bridge\n"));
	Sleep(1000);
	//##END OF CRITICAL REGION##
	WaitForSingleObject(param->meL2R, INFINITE);
	param->nR--;
	if (param->nR == 0)
		ReleaseSemaphore(semW, 1, NULL);
	ReleaseSemaphore(param->meL2R, 1, NULL);
	return 0;
}
DWORD WINAPI CARr2lThreads(LPVOID lpParam) {
	r2l_t* param = (r2l_t*)lpParam;
	WaitForSingleObject(param->meR2L, INFINITE);
	param->nR++;
	if (param->nR == 1)
		WaitForSingleObject(semW, INFINITE);
	ReleaseSemaphore(param->meR2L, 1, NULL);
	//##CRITICAL REGION##
	_tprintf(_T("CAR R2L Crossing the bridge\n"));
	Sleep(1000);
	//##END OF CRITICAL REGION##
	WaitForSingleObject(param->meR2L, INFINITE);
	param->nR--;
	if (param->nR == 0)
		ReleaseSemaphore(semW, 1, NULL);
	ReleaseSemaphore(param->meR2L, 1, NULL);
	return 0;
}
