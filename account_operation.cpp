#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<tchar.h>
#include<stdlib.h>
#include<stdio.h>
#define MAX_NAME_LEN 30+1

#define FL 1
#define CS 0
#define MT 0
#define SE 0


typedef struct account {
	DWORD id;
	DWORD account_no;
	TCHAR sur_name[MAX_NAME_LEN];
	TCHAR name[MAX_NAME_LEN];
	DWORD balance;
}accounts_t;

typedef struct syn_obj {
	HANDLE mutex, sem;
	CRITICAL_SECTION cs;
}syn_obj_t;
typedef struct thread {
	DWORD id;
	LPTSTR accountFile;
	LPTSTR operationFile;

}threads_t;

accounts_t* accounts;
threads_t* threadData;
syn_obj_t syn_obj;
DWORD N;

DWORD WINAPI threadFunctionFL(LPVOID);
DWORD WINAPI threadFunctionCS(LPVOID);
DWORD WINAPI threadFunctionMT(LPVOID);
DWORD WINAPI threadFunctionSE(LPVOID);

int _tmain(int argc, LPTSTR argv[]) {
	N = argc - 1;
	HANDLE* hThreads;
	hThreads = (HANDLE*)malloc(N-1 * sizeof(HANDLE));
	threadData = (threads_t*)malloc(N-1 * sizeof(threads_t));
	
	InitializeCriticalSection(&syn_obj.cs);
	syn_obj.mutex = CreateMutex(NULL, FALSE, NULL);
	syn_obj.sem = CreateSemaphore(NULL, 1, 1, NULL);

	INT i;
	for (i = 0; i < N - 1; i++) {
		threadData[i].id = i;
		threadData[i].accountFile = argv[1];
		threadData[i].operationFile = argv[i + 2];
#ifdef FL
		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunctionFL, &threadData[i], 0, NULL);
#endif // FL
#ifdef CS
		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunctionCS, &threadData[i], 0, NULL);
#endif // CS
#ifdef MT
		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunctionMT, &threadData[i], 0, NULL);
#endif // MT
#ifdef SE
		hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunctionSE, &threadData[i], 0, NULL);
#endif // SE
	
	}
		
	
}

DWORD WINAPI threadFunctionFL(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hAccount, hOperation; DWORD nIn;
	accounts_t accountData, operationData;
	
	hAccount = CreateFile(data->accountFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOperation = CreateFile(data->operationFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	OVERLAPPED ov = { 0,0,0,0, NULL };
	LARGE_INTEGER filePos, fileReserved;
	fileReserved.QuadPart = 1 * sizeof(accounts_t);

	while (hOperation, &operationData, sizeof(accounts_t), &nIn, NULL) {
		filePos.QuadPart = (operationData.id - 1) * sizeof(accounts_t);
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		ov.hEvent = 0;

		LockFileEx(hAccount, LOCKFILE_EXCLUSIVE_LOCK, 0, fileReserved.LowPart, fileReserved.HighPart, &ov);
			ReadFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
			accountData.balance += operationData.balance;
			WriteFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
		UnlockFileEx(hAccount, 0, fileReserved.LowPart, fileReserved.HighPart, &ov);
	}
	CloseHandle(hAccount);
	CloseHandle(hOperation);
	ExitThread(0);
}


DWORD WINAPI threadFunctionCS(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hAccount, hOperation; DWORD nIn;
	accounts_t accountData, operationData;

	hAccount = CreateFile(data->accountFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOperation = CreateFile(data->operationFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	OVERLAPPED ov = { 0,0,0,0, NULL };
	LARGE_INTEGER filePos;

	while (hOperation, &operationData, sizeof(accounts_t), &nIn, NULL) {
		filePos.QuadPart = (operationData.id - 1) * sizeof(accounts_t);
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		ov.hEvent = 0;
		EnterCriticalSection(&syn_obj.cs);
			ReadFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
			accountData.balance += operationData.balance;
			WriteFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
		LeaveCriticalSection(&syn_obj.cs);
	}
	CloseHandle(hAccount);
	CloseHandle(hOperation);
	ExitThread(0);
}
DWORD WINAPI threadFunctionMT(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hAccount, hOperation; DWORD nIn;
	accounts_t accountData, operationData;

	hAccount = CreateFile(data->accountFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOperation = CreateFile(data->operationFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	OVERLAPPED ov = { 0,0,0,0, NULL };
	LARGE_INTEGER filePos;

	while (hOperation, &operationData, sizeof(accounts_t), &nIn, NULL) {
		filePos.QuadPart = (operationData.id - 1) * sizeof(accounts_t);
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		ov.hEvent = 0;
		WaitForSingleObject(syn_obj.mutex, INFINITE);
			ReadFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
			accountData.balance += operationData.balance;
			WriteFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
		ReleaseMutex(syn_obj.mutex);
	}
	CloseHandle(hAccount);
	CloseHandle(hOperation);
	ExitThread(0);
}
DWORD WINAPI threadFunctionSE(LPVOID lpParam) {
	threads_t* data = (threads_t*)lpParam;
	HANDLE hAccount, hOperation; DWORD nIn;
	accounts_t accountData, operationData;

	hAccount = CreateFile(data->accountFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOperation = CreateFile(data->operationFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	OVERLAPPED ov = { 0,0,0,0, NULL };
	LARGE_INTEGER filePos;

	while (hOperation, &operationData, sizeof(accounts_t), &nIn, NULL) {
		filePos.QuadPart = (operationData.id - 1) * sizeof(accounts_t);
		ov.Offset = filePos.LowPart;
		ov.OffsetHigh = filePos.HighPart;
		ov.hEvent = 0;
		WaitForSingleObject(syn_obj.sem, INFINITE);
			ReadFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
			accountData.balance += operationData.balance;
			WriteFile(hAccount, &accountData, sizeof(accounts_t), &nIn, &ov);
		ReleaseSemaphore(syn_obj.sem, 1, NULL);
	}
	CloseHandle(hAccount);
	CloseHandle(hOperation);
	ExitThread(0);
}