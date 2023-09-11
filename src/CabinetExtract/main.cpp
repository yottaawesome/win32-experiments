#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <fdi.h>

#pragma comment(lib,"cabinet.lib")

//Function prototypes
BOOL ExtractCabinetFiles(
    LPSTR pszCabinetName,
    LPSTR pszCabinetPath,
    LPSTR pszDestinationDir
);
LPCSTR FDIErrorToString(FDIERROR err);

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnalloc
FNALLOC(fnMemAlloc)
{
    return malloc(cb);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnfree
FNFREE(fnMemFree)   //function to free memory
{
    free(pv);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnopen
FNOPEN(fnFileOpen)  //function to open a file
{
    HANDLE hFile = NULL;
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDisposition = 0;

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

    return (INT_PTR)hFile;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnread
FNREAD(fnFileRead)  //function to read data from a file
{
    DWORD dwBytesRead = 0;

    if (ReadFile((HANDLE)hf, pv, cb, &dwBytesRead, NULL) == FALSE)
    {
        dwBytesRead = (DWORD)-1L;
    }

    return dwBytesRead;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnwrite
FNWRITE(fnFileWrite) //function to write data to a file
{
    DWORD dwBytesWritten = 0;

    if (WriteFile((HANDLE)hf, pv, cb, &dwBytesWritten, NULL) == FALSE)
    {
        dwBytesWritten = (DWORD)-1;
    }

    return dwBytesWritten;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnclose
FNCLOSE(fnFileClose) //function to close a file
{
    return (CloseHandle((HANDLE)hf) == TRUE) ? 0 : -1;
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnseek
FNSEEK(fnFileSeek)  //function to move the file pointer
{
    return SetFilePointer((HANDLE)hf, dist, NULL, seektype);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fnfdinotify
FNFDINOTIFY(fnNotify)
{
    return 0;
}

int main(INT argc, CHAR* argv[])
{
    INT iExitCode = -1;
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    LPSTR pszCabinetName = NULL;    //the cabinet name
    LPSTR pszDestinationDir = NULL; //the destination directory
    CHAR pszCabinetPath[MAX_PATH];  //the cabinet path

    WIN32_FIND_DATAA findFileData;

    (VOID)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (argc != 3)
    {
        printf("Usage: %s [cabinet] [destination]\n", argv[0]);
        goto CLEANUP;
    }

    //Split the cabinet name and path

    pszCabinetName = strrchr(argv[1], '\\');

    if (pszCabinetName == NULL)
    {
        pszCabinetName = argv[1];
        pszCabinetPath[0] = '\0';
    }
    else
    {
        pszCabinetName++;
        hr = StringCchCopyA(pszCabinetPath, MAX_PATH, argv[1]);

        if (SUCCEEDED(hr))
        {
            pszCabinetPath[(INT)(pszCabinetName - argv[1])] = '\0';
        }
        else
        {
            printf("Failed to split the cabinet name \"%s\".\n",
                argv[1]);
            goto CLEANUP;
        }
    }

    pszDestinationDir = argv[2]; //Destination directory

    //Determine whether the destination directory provided by the user exists

    hFile = FindFirstFileA(pszDestinationDir, &findFileData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Destination directory \"%s\" does not exist.\n",
            pszDestinationDir);
        goto CLEANUP;
    }

    if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FALSE)
    {
        printf("Destination, \"%s\", is not a directory.\n",
            pszDestinationDir);
        goto CLEANUP;
    }

    //Get the files from the cabinet

    if (ExtractCabinetFiles(pszCabinetName, pszCabinetPath, pszDestinationDir) == TRUE)
    {
        iExitCode = 0;
    }

CLEANUP:

    if (hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
    }

    return iExitCode;
}

BOOL ExtractCabinetFiles(
    LPSTR pszCabinetName,
    LPSTR pszCabinetPath,
    LPSTR pszDestinationDir
)
{
    ERF  erf;         //FDI error structure
    HFDI hfdi = NULL; //FDI handle
    BOOL bSuccessful = FALSE;

    //Creates the FDI context

    // https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fdicreate
    hfdi = FDICreate(fnMemAlloc,  //function to allocate memory
        fnMemFree,   //function to free memory
        fnFileOpen,  //function to open a file
        fnFileRead,  //function to read data from a file
        fnFileWrite, //function to write data to a file
        fnFileClose, //function to close a file
        fnFileSeek,  //function to move the file pointer
        cpuUNKNOWN,  //only used by the 16-bit version of FDI
        &erf);       //pointer the FDI error structure

    if (hfdi == NULL)
    {
        printf("FDICreate failed with error code %d: %s\n",
            erf.erfOper,
            FDIErrorToString((FDIERROR)erf.erfOper));
        goto CLEANUP;
    }

    //Extract the files from the cabinet
    // https://learn.microsoft.com/en-us/windows/win32/api/fdi/nf-fdi-fdicopy
    if (FDICopy(hfdi,                        //FDI handle
        pszCabinetName,              //the cabinet name
        pszCabinetPath,              //the cabinet path
        0,                           //not used, set to zero
        fnNotify,                    //function for notifications
        NULL,                        //not used, set to NULL
        pszDestinationDir) == FALSE)//this value is application-specific 
    {
        printf("FDICopy failed with error code %d: %s\n",
            erf.erfOper,
            FDIErrorToString((FDIERROR)erf.erfOper));
        goto CLEANUP;
    }

    bSuccessful = TRUE;

CLEANUP:

    //Destory the FDI context

    if (hfdi != NULL)
    {
        if (FDIDestroy(hfdi) != TRUE)
        {
            printf("FDIDestroy failed with error code %d: %s\n",
                erf.erfOper,
                FDIErrorToString((FDIERROR)erf.erfOper));
        }
    }

    return bSuccessful;
}

LPCSTR FDIErrorToString(FDIERROR err)
{
    switch (err)
    {
    case FDIERROR_NONE:
        return "No error";

    case FDIERROR_CABINET_NOT_FOUND:
        return "Cabinet not found";

    case FDIERROR_NOT_A_CABINET:
        return "Not a cabinet";

    case FDIERROR_UNKNOWN_CABINET_VERSION:
        return "Unknown cabinet version";

    case FDIERROR_CORRUPT_CABINET:
        return "Corrupt cabinet";

    case FDIERROR_ALLOC_FAIL:
        return "Memory allocation failed";

    case FDIERROR_BAD_COMPR_TYPE:
        return "Unknown compression type";

    case FDIERROR_MDI_FAIL:
        return "Failure decompressing data";

    case FDIERROR_TARGET_FILE:
        return "Failure writing to target file";

    case FDIERROR_RESERVE_MISMATCH:
        return "Cabinets in set have different RESERVE sizes";

    case FDIERROR_WRONG_CABINET:
        return "Cabinet returned on fdintNEXT_CABINET is incorrect";

    case FDIERROR_USER_ABORT:
        return "Application aborted";

    default:
        return "Unknown error";
    }
}
