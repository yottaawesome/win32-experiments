#pragma once
#include <windows.h>
#include <lmcons.h>

// Based on https://docs.microsoft.com/en-us/windows/win32/netmgmt/creating-a-local-group-and-adding-a-user
// Need to run this from an elevated process
NET_API_STATUS NetSample(const wchar_t* lpszDomain,
    const wchar_t* lpszUser,
    const wchar_t* lpszPassword,
    const wchar_t* lpszLocalGroup);

NET_API_STATUS NetSampleNoDc(
    const wchar_t* lpszUser,
    const wchar_t* lpszPassword,
    const wchar_t* lpszLocalGroup);

void DeleteLocalUserAndGroup
(
    const wchar_t* userName,
    const wchar_t* groupName
);

DWORD CreateLocalGroup(
    const wchar_t* groupName,
    const wchar_t* comment
);

void LogonLocalUser(const wchar_t* lpszUser, const wchar_t* lpszPassword);
