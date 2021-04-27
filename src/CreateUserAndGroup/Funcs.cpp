#include "Funcs.hpp"
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <stdexcept>
#include <iostream>

#pragma comment(lib, "netapi32.lib")

NET_API_STATUS NetSample(const wchar_t* lpszDomain,
    const wchar_t* lpszUser,
    const wchar_t* lpszPassword,
    const wchar_t* lpszLocalGroup)
{

    LOCALGROUP_INFO_1         localgroup_info;
    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    LPWSTR                    lpszPrimaryDC = nullptr;
    NET_API_STATUS            err = 0;
    DWORD                     parm_err = 0;

    // First get the name of the primary domain controller. 
    // Be sure to free the returned buffer. 

    err = NetGetDCName(
        NULL,                    // local computer 
        lpszDomain,                   // domain name 
        (LPBYTE*)&lpszPrimaryDC);  // returned PDC 
    if (err != 0)
    {
        std::wcout << "Error getting DC name: " << err << std::endl;
        return(err);

    }

    // Set up the USER_INFO_1 structure. 
    USER_INFO_1               user_info;
    user_info.usri1_name = LPWSTR(lpszUser);
    user_info.usri1_password = LPWSTR(lpszPassword);
    user_info.usri1_priv = USER_PRIV_USER;
    user_info.usri1_home_dir = LPWSTR(L"");
    user_info.usri1_comment = LPWSTR(L"Sample User");
    user_info.usri1_flags = UF_SCRIPT;
    user_info.usri1_script_path = LPWSTR(L"");

    err = NetUserAdd(lpszPrimaryDC,        // PDC name 
        1,                    // level 
        (LPBYTE)&user_info,  // input buffer 
        &parm_err);          // parameter in error 

    switch (err)
    {
    case 0:
        std::wcout << "User successfully created.\n";
        break;
    case NERR_UserExists:
        std::wcout << "User already exists: " << err << std::endl;
        err = 0;
        break;
    case ERROR_INVALID_PARAMETER:
        std::wcout << "Invalid parameter error adding user " << err << std::endl;
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    default:
        std::wcout << "Error adding user: " << err << std::endl;
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    }

    // Set up the LOCALGROUP_INFO_1 structure. 

    localgroup_info.lgrpi1_name = LPWSTR(lpszLocalGroup);
    localgroup_info.lgrpi1_comment = LPWSTR(L"Sample local group.");

    err = NetLocalGroupAdd(lpszPrimaryDC,    // PDC name 
        1,                          // level 
        (LPBYTE)&localgroup_info,  // input buffer 
        &parm_err);                // parameter in error 

    switch (err)
    {
    case 0:
        std::wcout << ("Local group successfully created.\n");
        break;
    case ERROR_ALIAS_EXISTS:
        std::wcout << ("Local group already exists.\n");
        err = 0;
        break;
    case ERROR_INVALID_PARAMETER:
        std::wcout << "Invalid parameter error adding local group; parameter index " << err << std::endl;
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    default:
        std::wcout << "Error adding local group: " << err << std::endl;
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    }

    // Now add the user to the local group. 

    localgroup_members.lgrmi3_domainandname = LPWSTR(lpszUser);

    err = NetLocalGroupAddMembers(lpszPrimaryDC,        // PDC name 
        lpszLocalGroup,               // group name 
        3,                            // name 
        (LPBYTE)&localgroup_members, // buffer 
        1);                          // count

    switch (err)
    {
    case 0:
        std::wcout << "User successfully added to local group" << std::endl;
        break;
    case ERROR_MEMBER_IN_ALIAS:
        std::wcout << "User already in local group" << std::endl;
        err = 0;
        break;
    default:
        std::wcout << "Error adding user to local group: " << err << std::endl;
        break;
    }

    NetApiBufferFree(lpszPrimaryDC);
    return(err);
}

NET_API_STATUS NetSampleNoDc(
    const wchar_t* lpszUser,
    const wchar_t* lpszPassword,
    const wchar_t* lpszLocalGroup)
{

    LOCALGROUP_INFO_1         localgroup_info;
    LOCALGROUP_MEMBERS_INFO_3 localgroup_members;
    NET_API_STATUS            err = 0;
    DWORD                     parm_err = 0;

    // Set up the USER_INFO_1 structure. 
    USER_INFO_1               user_info;
    user_info.usri1_name = LPWSTR(lpszUser);
    user_info.usri1_password = LPWSTR(lpszPassword);
    user_info.usri1_priv = USER_PRIV_USER;
    user_info.usri1_home_dir = LPWSTR(L"");
    user_info.usri1_comment = LPWSTR(L"Sample User");
    user_info.usri1_flags = UF_SCRIPT;
    user_info.usri1_script_path = LPWSTR(L"");

    err = NetUserAdd(
        nullptr,        // PDC name 
        1,                    // level 
        (LPBYTE)&user_info,  // input buffer 
        &parm_err);          // parameter in error 

    switch (err)
    {
        case 0:
            std::wcout << "User successfully created.\n";
            break;
        case NERR_UserExists:
            std::wcout << ("User already exists.\n");
            err = 0;
            break;
        case ERROR_INVALID_PARAMETER:
            std::wcout << "Invalid parameter error adding user; parameter index = " << parm_err << std::endl;
            return(err);
        default:
            std::wcout << "Error adding user " << err << std::endl;
            return(err);
    }

    // Set up the LOCALGROUP_INFO_1 structure. 

    localgroup_info.lgrpi1_name = LPWSTR(lpszLocalGroup);
    localgroup_info.lgrpi1_comment = LPWSTR(L"Sample local group.");

    err = NetLocalGroupAdd(
        nullptr,    // PDC name 
        1,                          // level 
        (LPBYTE)&localgroup_info,  // input buffer 
        &parm_err);                // parameter in error 

    switch (err)
    {
        case 0:
            std::wcout << ("Local group successfully created.\n");
            break;
        case ERROR_ALIAS_EXISTS:
            std::wcout << ("Local group already exists.\n");
            err = 0;
            break;
        case ERROR_INVALID_PARAMETER:
            std::wcout << "Invalid parameter error adding local group; parameter index = " << parm_err << std::endl;
            return(err);
        default:
            std::wcout << "Error adding local group: " << err << std::endl;
            return(err);
    }

    // Now add the user to the local group. 

    localgroup_members.lgrmi3_domainandname = LPWSTR(lpszUser);

    err = NetLocalGroupAddMembers(
        nullptr,        // PDC name 
        lpszLocalGroup,               // group name 
        3,                            // name 
        (LPBYTE)&localgroup_members, // buffer 
        1);                          // count

    switch (err)
    {
        case 0:
            std::wcout << ("User successfully added to local group.\n");
            break;
        case ERROR_MEMBER_IN_ALIAS:
            std::wcout << ("User already in local group.\n");
            err = 0;
            break;
        default:
            std::wcout << "Error adding user to local group: " << err << std::endl;
            break;
    }

    return(err);
}

DWORD CreateLocalGroup(
    const wchar_t* groupName,
    const wchar_t* comment
)
{
    LOCALGROUP_INFO_1         localgroup_info;
    // Set up the LOCALGROUP_INFO_1 structure. 

    localgroup_info.lgrpi1_name = LPWSTR(groupName);
    localgroup_info.lgrpi1_comment = LPWSTR(L"Sample local group.");

    DWORD parm_err = 0;
    DWORD err = NetLocalGroupAdd(
        nullptr,    // PDC name 
        1,                          // level 
        (LPBYTE)&localgroup_info,  // input buffer 
        &parm_err);                // parameter in error 

    switch (err)
    {
        case 0:
            std::wcout << ("Local group successfully created.\n");
            break;
        
        case ERROR_ALIAS_EXISTS:
            std::wcout << ("Local group already exists.\n");
            err = 0;
            break;
     
        case ERROR_INVALID_PARAMETER:
            std::wcout << "Invalid parameter error adding local group; parameter index = " << parm_err << std::endl;
            break;
        
        default:
            std::wcout << "Error adding local group: " << err << std::endl;
            break;
    }
    return err;
}

void DeleteLocalUserAndGroup
(
    const wchar_t* userName,
    const wchar_t* groupName
) 
{
    if (NetUserDel(nullptr, userName) != NERR_Success)
        throw std::runtime_error("NetUserDel failed");
    //NetGroupDel does not work on local systems with no DC (?)
    DWORD op2 = NetLocalGroupDel(nullptr, groupName);
    if (op2 != NERR_Success)
        throw std::runtime_error("NetGroupDel failed");

    //see also 
    //https://docs.microsoft.com/en-us/windows/win32/api/lmaccess/nf-lmaccess-netlocalgroupdel
    //https://docs.microsoft.com/en-us/windows/win32/api/lmaccess/nf-lmaccess-netgroupdeluser
}

void LogonLocalUser(const wchar_t* lpszUser, const wchar_t* lpszPassword)
{
    HANDLE outToken = nullptr;
    bool successful = LogonUserW(
        lpszUser,
        L".",
        lpszPassword,
        LOGON32_LOGON_INTERACTIVE,
        LOGON32_PROVIDER_DEFAULT,
        &outToken
    );

    if (successful)
        std::wcout << "Logon OK" << std::endl;
    else
        std::wcout << "Logon not OK" << std::endl;

    if (outToken)
        CloseHandle(outToken);
}