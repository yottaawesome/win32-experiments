// Adapted from https://learn.microsoft.com/en-us/windows/win32/wes/formatting-event-messages

#include <windows.h>
#include <stdio.h>
#include <sddl.h>
#include <winevt.h>

#pragma comment(lib, "wevtapi.lib")

LPWSTR GetMessageString(EVT_HANDLE hMetadata, EVT_HANDLE hEvent, EVT_FORMAT_MESSAGE_FLAGS FormatId);

int main(int argc, char* argv[])
{
    EVT_HANDLE hProviderMetadata = NULL;
    EVT_HANDLE hResults = NULL;
    EVT_HANDLE hEvent = NULL;
    DWORD status = ERROR_SUCCESS;
    DWORD dwReturned = 0;
    LPWSTR pwsMessage = NULL;
    LPWSTR pwsPath = (LPWSTR)L"<name of the channel goes here>";
    LPWSTR pwsQuery = (LPWSTR)L"<xpath query goes here>";
    LPWSTR pwszPublisherName = (LPWSTR)L"<name of the publisher goes here>";

    // Get the handle to the provider's metadata that contains the message strings.
    hProviderMetadata = EvtOpenPublisherMetadata(NULL, pwszPublisherName, NULL, 0, 0);
    if (NULL == hProviderMetadata)
    {
        wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
        goto cleanup;
    }

    // Query for an event.
    hResults = EvtQuery(NULL, pwsPath, pwsQuery, EvtQueryChannelPath);
    if (NULL == hResults)
    {
        status = GetLastError();

        if (ERROR_EVT_CHANNEL_NOT_FOUND == status)
            wprintf(L"Channel %s was not found.\n", pwsPath);
        else
            wprintf(L"EvtQuery failed with %lu.\n", status);

        goto cleanup;
    }

    // Get a single event from the result set.
    if (!EvtNext(hResults, 1, &hEvent, INFINITE, 0, &dwReturned))
    {
        wprintf(L"EvtNext failed with %lu\n", status);
        goto cleanup;
    }

    // Get the various message strings from the event.
    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageEvent);
    if (pwsMessage)
    {
        wprintf(L"Event message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageLevel);
    if (pwsMessage)
    {
        wprintf(L"Level message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageTask);
    if (pwsMessage)
    {
        wprintf(L"Task message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageOpcode);
    if (pwsMessage)
    {
        wprintf(L"Opcode message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageKeyword);
    if (pwsMessage)
    {
        LPWSTR ptemp = pwsMessage;

        wprintf(L"Keyword message string: %s", ptemp);

        while (*(ptemp += wcslen(ptemp) + 1))
            wprintf(L", %s", ptemp);

        wprintf(L"\n\n");
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageChannel);
    if (pwsMessage)
    {
        wprintf(L"Channel message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageProvider);
    if (pwsMessage)
    {
        wprintf(L"Provider message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageXml);
    if (pwsMessage)
    {
        wprintf(L"XML message string: %s\n\n", pwsMessage);
        free(pwsMessage);
        pwsMessage = NULL;
    }

cleanup:

    if (hEvent)
        EvtClose(hEvent);

    if (hResults)
        EvtClose(hResults);

    if (hProviderMetadata)
        EvtClose(hProviderMetadata);

    return 0;
}


// Gets the specified message string from the event. If the event does not
// contain the specified message, the function returns NULL.
LPWSTR GetMessageString(EVT_HANDLE hMetadata, EVT_HANDLE hEvent, EVT_FORMAT_MESSAGE_FLAGS FormatId)
{
    LPWSTR pBuffer = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD status = 0;

    if (!EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed))
    {
        status = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            // An event can contain one or more keywords. The function returns keywords
            // as a list of keyword strings. To process the list, you need to know the
            // size of the buffer, so you know when you have read the last string, or you
            // can terminate the list of strings with a second null terminator character 
            // as this example does.
            if ((EvtFormatMessageKeyword == FormatId))
                pBuffer[dwBufferSize - 1] = L'\0';
            else
                dwBufferSize = dwBufferUsed;

            pBuffer = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));

            if (pBuffer)
            {
                EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed);

                // Add the second null terminator character.
                if ((EvtFormatMessageKeyword == FormatId))
                    pBuffer[dwBufferUsed - 1] = L'\0';
            }
            else
            {
                wprintf(L"malloc failed\n");
            }
        }
        else if (ERROR_EVT_MESSAGE_NOT_FOUND == status || ERROR_EVT_MESSAGE_ID_NOT_FOUND == status)
            ;
        else
        {
            wprintf(L"EvtFormatMessage failed with %u\n", status);
        }
    }

    return pBuffer;
}