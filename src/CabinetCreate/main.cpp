#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <fdi.h>
#include <fci.h>

#pragma comment(lib,"cabinet.lib")

//Function prototypes
BOOL InitCab(PCCAB pccab);
LPCSTR FCIErrorToString(FCIERROR err);

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcifileplaced
FNFCIFILEPLACED(fnFilePlaced)
{
    return 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcialloc
FNFCIALLOC(fnMemAlloc)        //function to allocate memory
{
    return malloc(cb);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcifree
FNFCIFREE(fnMemFree)         //function to free memory
{
    free(memory);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfciopen
FNFCIOPEN(fnFileOpen)        //function to open a file
{
    HANDLE hFile = NULL;
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDisposition = 0;

    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(pmode);

    if (oflag & _O_RDWR)
    {
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    }
    else if (oflag & _O_WRONLY)
    {
        dwDesiredAccess = GENERIC_WRITE;
    }
    else
    {
        dwDesiredAccess = GENERIC_READ;
    }

    if (oflag & _O_CREAT)
    {
        dwCreationDisposition = CREATE_ALWAYS;
    }
    else
    {
        dwCreationDisposition = OPEN_EXISTING;
    }

    hFile = CreateFileA(pszFile,
        dwDesiredAccess,
        FILE_SHARE_READ,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        *err = GetLastError();
    }

    return (INT_PTR)hFile;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfciread
FNFCIREAD(fnFileRead)        //function to read data from a file
{
    DWORD dwBytesRead = 0;

    UNREFERENCED_PARAMETER(pv);

    if (ReadFile((HANDLE)hf, memory, cb, &dwBytesRead, NULL) == FALSE)
    {
        dwBytesRead = (DWORD)-1;
        *err = GetLastError();
    }

    return dwBytesRead;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfciwrite
FNFCIWRITE(fnFileWrite)       //function to write data to a file
{
    DWORD dwBytesWritten = 0;

    UNREFERENCED_PARAMETER(pv);

    if (WriteFile((HANDLE)hf, memory, cb, &dwBytesWritten, NULL) == FALSE)
    {
        dwBytesWritten = (DWORD)-1;
        *err = GetLastError();
    }

    return dwBytesWritten;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfciclose
FNFCICLOSE(fnFileClose) //function to close a file
{
    INT iResult = 0;

    UNREFERENCED_PARAMETER(pv);

    if (CloseHandle((HANDLE)hf) == FALSE)
    {
        *err = GetLastError();
        iResult = -1;
    }

    return iResult;
}       

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfciseek
FNFCISEEK(fnFileSeek)        //function to move the file pointer
{
    INT iResult = 0;

    UNREFERENCED_PARAMETER(pv);

    iResult = SetFilePointer((HANDLE)hf, dist, NULL, seektype);

    if (iResult == -1)
    {
        *err = GetLastError();
    }

    return iResult;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcidelete
FNFCIDELETE(fnFileDelete)      //function to delete a file
{
    INT iResult = 0;

    UNREFERENCED_PARAMETER(pv);

    if (DeleteFileA(pszFile) == FALSE)
    {
        *err = GetLastError();
        iResult = -1;
    }

    return iResult;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcigettempfile
FNFCIGETTEMPFILE(fnGetTempFileName) //function to obtain a temporary file name
{
    BOOL bSucceeded = FALSE;
    CHAR pszTempPath[MAX_PATH];
    CHAR pszTempFile[MAX_PATH];

    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(cbTempName);

    if (GetTempPathA(MAX_PATH, pszTempPath) != 0)
    {
        if (GetTempFileNameA(pszTempPath, "CABINET", 0, pszTempFile) != 0)
        {
            DeleteFileA(pszTempFile);
            bSucceeded = SUCCEEDED(StringCbCopyA(pszTempName, cbTempName, pszTempFile));
        }
    }

    return bSucceeded;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcigetnextcabinet
FNFCIGETNEXTCABINET(fnGetNextCabinet)
{
    HRESULT hr;

    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(cbPrevCab);

    hr = StringCchPrintfA(pccab->szCab,
        ARRAYSIZE(pccab->szCab),
        "FCISample%02d.cab",
        pccab->iCab);

    return (SUCCEEDED(hr));
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcistatus
FNFCISTATUS(fnStatus)
{
    return true;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fnfcigetopeninfo
FNFCIGETOPENINFO(fnGetOpenInfo)
{
    HANDLE hFile;
    FILETIME fileTime;
    BY_HANDLE_FILE_INFORMATION fileInfo;

    hFile = (HANDLE)fnFileOpen(pszName, _O_RDONLY, 0, err, pv);

    if (hFile != (HANDLE)-1)
    {
        if (GetFileInformationByHandle(hFile, &fileInfo)
            && FileTimeToLocalFileTime(&fileInfo.ftCreationTime, &fileTime)
            && FileTimeToDosDateTime(&fileTime, pdate, ptime))
        {
            *pattribs = (USHORT)fileInfo.dwFileAttributes;
            *pattribs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
        }
        else
        {
            fnFileClose((INT_PTR)hFile, err, pv);
            hFile = (HANDLE)-1;
        }
    }

    return (INT_PTR)hFile;
}

int main(INT argc, CHAR* argv[])
{
    ERF   erf;            //FCI error structure
    HFCI  hfci = NULL;    //FCI handle
    CCAB  ccab;           //cabinet information structure
    INT   iArg;           //Argument counter
    INT   iExitCode = -1; //The exit code
    LPSTR pszFileName;    //The file name to store in the cabinet

    (VOID)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    ZeroMemory(&erf, sizeof(ERF));

    if (argc < 2)
    {
        printf("Usage: %s [file1] [file2] [+] [file3] ...\n"
            "\n"
            "file - The file to be added to the cabinet.\n"
            "+    - Creates a new folder for the succeeding files.\n"
            , argv[0]);
        goto CLEANUP;
    }

    if (InitCab(&ccab) == FALSE)
    {
        printf("Failed to initialize the cabinet information structure.\n");
        goto CLEANUP;
    }

    //Creates the FCI context

    hfci = FCICreate(&erf,              //pointer the FCI error structure
        fnFilePlaced,      //function to call when a file is placed
        fnMemAlloc,        //function to allocate memory
        fnMemFree,         //function to free memory
        fnFileOpen,        //function to open a file
        fnFileRead,        //function to read data from a file
        fnFileWrite,       //function to write data to a file
        fnFileClose,       //function to close a file
        fnFileSeek,        //function to move the file pointer
        fnFileDelete,      //function to delete a file
        fnGetTempFileName, //function to obtain a temporary file name
        &ccab,             //pointer to the FCI cabinet information structure
        NULL);             //client context parameter, NULL for this sample.

    if (hfci == NULL)
    {
        printf("FCICreate failed with error code %d: %s\n",
            erf.erfOper,
            FCIErrorToString((FCIERROR)erf.erfOper));
        goto CLEANUP;
    }

    //Add the files to the cabinet

    for (iArg = 1; iArg < argc; iArg++)
    {
        if (strcmp(argv[iArg], "+") == 0)
        {
            // https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fciflushfolder
            if (FCIFlushFolder(hfci,                //FCI handle
                fnGetNextCabinet,    //function to get the next cabinet specifications
                fnStatus) == FALSE) //function to update the cabinet status

            {
                printf("FCIFlushFolder failed with error code %d: %s\n",
                    erf.erfOper,
                    FCIErrorToString((FCIERROR)erf.erfOper));
                goto CLEANUP;
            }
        }
        else
        {
            //Remove the directory structure from the file name to store

            pszFileName = strrchr(argv[iArg], '\\');

            if (pszFileName == NULL)
            {
                pszFileName = argv[iArg];
            }

            //Adds a file to the cabinet under construction
            // https://learn.microsoft.com/en-us/windows/win32/api/fci/nf-fci-fciaddfile
            if (FCIAddFile(hfci,                       //FCI handle
                argv[iArg],                 //file to add
                pszFileName,                //file name to store
                FALSE,                      //do not run when extracted
                fnGetNextCabinet,           //function to get the next cabinet specifications
                fnStatus,                   //function to update the cabinet status
                fnGetOpenInfo,              //function to get the file date, time and attributes
                tcompTYPE_MSZIP) == FALSE) //use MSZIP compression

            {
                printf("FCIAddFile failed with error code %d: %s\n",
                    erf.erfOper,
                    FCIErrorToString((FCIERROR)erf.erfOper));
                goto CLEANUP;
            }
        }
    }

    //Complete the cabinet

    if (FCIFlushCabinet(hfci,                //FCI handle
        FALSE,               //do not call fnGetNextCabinet
        fnGetNextCabinet,    //function to get the next cabinet specifications
        fnStatus) == FALSE) //function to update the cabinet status
    {
        printf("FCIFlushCabinet failed with error code %d: %s\n",
            erf.erfOper,
            FCIErrorToString((FCIERROR)erf.erfOper));
        goto CLEANUP;
    }

    iExitCode = 0;

CLEANUP:

    //Destory the FCI context

    if (hfci != NULL)
    {
        if (FCIDestroy(hfci) != TRUE)
        {
            printf("FCIDestroy failed with error code %d: %s\n",
                erf.erfOper,
                FCIErrorToString((FCIERROR)erf.erfOper));
        }
    }

    return iExitCode;
}

BOOL InitCab(PCCAB pccab)
{
    BOOL bInit = FALSE;
    DWORD dCurrentDir;
    HRESULT hr;

    ZeroMemory(pccab, sizeof(CCAB));

    pccab->cb = 0x100000;            //Maximum cabinet size in bytes
    pccab->cbFolderThresh = 0x10000; //Maximum folder size in bytes

    pccab->setID = 555; //Cabinet set ID
    pccab->iCab = 1;    //Number of this cabinet in a set
    pccab->iDisk = 0;   //Disk number


    if (fnGetNextCabinet(pccab, 0, NULL) == TRUE) //Get the next cabinet name
    {
        //Set the disk name to empty

        pccab->szDisk[0] = '\0';

        //Set the cabinet path to the current directory

        dCurrentDir = GetCurrentDirectoryA(ARRAYSIZE(pccab->szCabPath),
            pccab->szCabPath);

        if (dCurrentDir != 0)
        {
            hr = StringCchCatA(pccab->szCabPath,
                ARRAYSIZE(pccab->szCabPath),
                "\\");

            bInit = SUCCEEDED(hr);
        }
    }

    return bInit;
}

LPCSTR FCIErrorToString(FCIERROR err)
{
    switch (err)
    {
    case FCIERR_NONE:
        return "No error";

    case FCIERR_OPEN_SRC:
        return "Failure opening file to be stored in cabinet";

    case FCIERR_READ_SRC:
        return "Failure reading file to be stored in cabinet";

    case FCIERR_ALLOC_FAIL:
        return "Insufficient memory in FCI";

    case FCIERR_TEMP_FILE:
        return "Could not create a temporary file";

    case FCIERR_BAD_COMPR_TYPE:
        return "Unknown compression type";

    case FCIERR_CAB_FILE:
        return "Could not create cabinet file";

    case FCIERR_USER_ABORT:
        return "Client requested abort";

    case FCIERR_MCI_FAIL:
        return "Failure compressing data";

    default:
        return "Unknown error";
    }
}