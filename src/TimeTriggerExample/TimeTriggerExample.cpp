// https://docs.microsoft.com/en-us/windows/win32/taskschd/time-trigger-example--c---
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <comdef.h>
#include <wincred.h>
//  Include the task header file.
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "credui.lib")

std::wstring GetErrorFromHResult(const std::wstring& msg, const HRESULT hr) noexcept
{
    try
    {
        std::wstringstream ss;
        _com_error ce(hr);
        ss
            << msg
            << std::endl
            << ce.ErrorMessage()
            << L" (HRESULT: "
            << std::to_wstring(hr)
            << L")";
        return ss.str();
    }
    catch (const std::exception& ex)
    {
        std::wcerr << __FUNCSIG__ << ": " << ex.what() << std::endl;
        return L"";
    }
}

int main(int argc, char* args[])
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Failed", hr) << std::endl;
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create a name for the task.
    LPCWSTR wszTaskName = L"Time Trigger Test Task";

    //  Get the windows directory and set the path to notepad.exe.
    std::wstring wstrExecutablePath;
    // std::wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wchar_t* pValue = nullptr;
    size_t len;
    errno_t err = _wdupenv_s(&pValue, &len, L"WINDIR");
    if (err || pValue == nullptr) 
        return -1;
    wstrExecutablePath = pValue;
    free(pValue); // Sample uses free, and we can't mix between C++ and C mem allocation, so just free it
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Failed to create an instance of ITaskService", hr) << std::endl;
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"ITaskService::Connect failed", hr) << std::endl;
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.  This folder will hold the
    //  new task that is registered.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get Root folder pointer", hr) << std::endl;
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  If the same task exists, remove it.
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

    //  Create the task definition object to create the task.
    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Failed to CoCreate an instance of the TaskService class", hr) << std::endl;
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the registration info for setting the identification.
    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get identification pointer", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author(_bstr_t(L"Author Name"));
    pRegInfo->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put identification info", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create the principal for the task - these credentials
    //  are overwritten with the credentials passed to RegisterTaskDefinition
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get principal pointer", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set up principal logon type to interactive logon
    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    pPrincipal->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put principal info", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create the settings for the task
    ITaskSettings* pSettings = NULL;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get settings pointer", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task.  
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put setting information", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    // Set the idle settings for the task.
    IIdleSettings* pIdleSettings = NULL;
    hr = pSettings->get_IdleSettings(&pIdleSettings);
    pSettings->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get idle setting information", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pIdleSettings->put_WaitTimeout(bstr_t(L"PT5M"));
    pIdleSettings->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put idle setting information", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the trigger collection to insert the time trigger.
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get trigger collection", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the time trigger to the task.
    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot create trigger", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    ITimeTrigger* pTimeTrigger = NULL;
    hr = pTrigger->QueryInterface(
        IID_ITimeTrigger, (void**)&pTimeTrigger);
    pTrigger->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"QueryInterface call failed for ITimeTrigger", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pTimeTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put trigger ID", hr) << std::endl;
    }
        
    hr = pTimeTrigger->put_EndBoundary(_bstr_t(L"2015-05-02T08:00:00"));
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put end boundary on trigger", hr) << std::endl;
    }

    //  Set the task to start at a certain time. The time 
    //  format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    //  For example, the start boundary below
    //  is January 1st 2005 at 12:05
    hr = pTimeTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
    pTimeTrigger->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot add start boundary to trigger", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }


    //  ------------------------------------------------------
    //  Add an action to the task. This task will execute notepad.exe.     
    IActionCollection* pActionCollection = NULL;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot get Task collection pointer", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot create the action", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction* pExecAction = NULL;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(
        IID_IExecAction, (void**)&pExecAction);
    pAction->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"QueryInterface call failed for IExecAction", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Cannot put action path", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Save the task in the root folder.
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(wszTaskName),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(),
        _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(L""),
        &pRegisteredTask
    );
    if (FAILED(hr))
    {
        std::wcerr << GetErrorFromHResult(L"Error saving the Task", hr) << std::endl;
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    std::wcout << L"Success! Task successfully registered." << std::endl;

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
