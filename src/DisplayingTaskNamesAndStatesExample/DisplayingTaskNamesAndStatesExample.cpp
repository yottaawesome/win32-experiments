// https://docs.microsoft.com/en-us/windows/win32/taskschd/displaying-task-names-and-state--c---

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
//  Include the task header file.
#include <taskschd.h>
#include <wrl/client.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

using Microsoft::WRL::ComPtr;

int main()
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
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

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
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

    pService->Release();
    if (FAILED(hr))
    {
        printf("Cannot get Root Folder pointer: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  -------------------------------------------------------
    //  Get the registered tasks in the folder.
    IRegisteredTaskCollection* pTaskCollection = NULL;
    hr = pRootFolder->GetTasks(NULL, &pTaskCollection);

    pRootFolder->Release();
    if (FAILED(hr))
    {
        printf("Cannot get the registered tasks.: %x", hr);
        CoUninitialize();
        return 1;
    }

    LONG numTasks = 0;
    hr = pTaskCollection->get_Count(&numTasks);

    if (numTasks == 0)
    {
        printf("\nNo Tasks are currently running");
        pTaskCollection->Release();
        CoUninitialize();
        return 1;
    }

    printf("\nNumber of Tasks : %d", numTasks);

    TASK_STATE taskState;

    for (LONG i = 0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);

        ComPtr<ITaskDefinition> taskDefinition;
        pRegisteredTask->get_Definition(taskDefinition.GetAddressOf());
        ComPtr<ITaskSettings> settings;
        taskDefinition->get_Settings(settings.GetAddressOf());
        VARIANT_BOOL startWhenAvailable;
        settings->get_StartWhenAvailable(&startWhenAvailable);
        printf("\nStartWhenAvailable: %d", startWhenAvailable);
        ComPtr<ITriggerCollection> triggers;
        taskDefinition->get_Triggers(triggers.GetAddressOf());
        long count = 0;
        triggers->get_Count(&count);
        for (int i = 1; i <= count; i++) // Collections start at 1
        {
            ComPtr<ITrigger> trigger = nullptr;
            triggers->get_Item(i, &trigger);
            TASK_TRIGGER_TYPE2 type;
            trigger->get_Type(&type);
            if (type == TASK_TRIGGER_DAILY)
            {
                printf("\nThis is a daily trigger");
                IDailyTrigger* dailyTrigger = (IDailyTrigger*)trigger.Get();
                short days = 0;
                dailyTrigger->get_DaysInterval(&days);
                printf("\nDays: %d", days);
                //IRepetitionPattern* pattern = nullptr;
                //pattern->
            }
        }
        

        if (SUCCEEDED(hr))
        {
            BSTR taskName = NULL;
            hr = pRegisteredTask->get_Name(&taskName);
            if (SUCCEEDED(hr))
            {
                printf("\nTask Name: %S", taskName);
                SysFreeString(taskName);

                hr = pRegisteredTask->get_State(&taskState);
                if (SUCCEEDED(hr))
                    printf("\n\tState: %d", taskState);
                else
                    printf("\n\tCannot get the registered task state: %x", hr);
            }
            else
            {
                printf("\nCannot get the registered task name: %x", hr);
            }
            pRegisteredTask->Release();
        }
        else
        {
            printf("\nCannot get the registered task item at index=%d: %x", i + 1, hr);
        }
    }

    pTaskCollection->Release();
    CoUninitialize();
    return 0;

    return 0;
}

