/* A program to demonstrate the use of guard pages of memory. Allocate
   a page of memory as a guard page, then try to access the page. That
   will fail, but doing so releases the lock on the guard page, so the
   next access works correctly.

   The output will look like this. The actual address may vary.

   This computer has a page size of 4096.
   Committed 4096 bytes at address 0x00520000
   Cannot lock at 00520000, error = 0x80000001
   2nd Lock Achieved at 00520000

   This sample does not show how to use the guard page fault to
   "grow" a dynamic array, such as a stack. */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

int main()
{
    LPVOID lpvAddr;               // address of the test memory
    DWORD dwPageSize;             // amount of memory to allocate.
    BOOL bLocked;                 // address of the guarded memory
    SYSTEM_INFO sSysInfo;         // useful information about the system

    GetSystemInfo(&sSysInfo);     // initialize the structure

    _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);

    dwPageSize = sSysInfo.dwPageSize;

    // Try to allocate the memory.

    lpvAddr = VirtualAlloc(NULL, dwPageSize,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READONLY | PAGE_GUARD);

    if (lpvAddr == NULL) {
        _tprintf(TEXT("VirtualAlloc failed. Error: %ld\n"),
            GetLastError());
        return 1;

    }
    else {
        _ftprintf(stderr, TEXT("Committed %lu bytes at address 0x%lp\n"),
            dwPageSize, lpvAddr);
    }

    // Try to lock the committed memory. This fails the first time 
    // because of the guard page.

    bLocked = VirtualLock(lpvAddr, dwPageSize);
    if (!bLocked) {
        _ftprintf(stderr, TEXT("Cannot lock at %lp, error = 0x%lx\n"),
            lpvAddr, GetLastError());
    }
    else {
        _ftprintf(stderr, TEXT("Lock Achieved at %lp\n"), lpvAddr);
    }

    // Try to lock the committed memory again. This succeeds the second
    // time because the guard page status was removed by the first 
    // access attempt.

    bLocked = VirtualLock(lpvAddr, dwPageSize);

    if (!bLocked) {
        _ftprintf(stderr, TEXT("Cannot get 2nd lock at %lp, error = %lx\n"),
            lpvAddr, GetLastError());
    }
    else {
        _ftprintf(stderr, TEXT("2nd Lock Achieved at %lp\n"), lpvAddr);
    }

    return 0;
}