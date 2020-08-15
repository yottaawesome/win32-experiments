#include "Funcs.hpp"
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

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
        printf("Error getting DC name: %d\n", err);
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
        printf("User successfully created.\n");
        break;
    case NERR_UserExists:
        printf("User already exists.\n");
        err = 0;
        break;
    case ERROR_INVALID_PARAMETER:
        printf("Invalid parameter error adding user; parameter index = %d\n",
            parm_err);
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    default:
        printf("Error adding user: %d\n", err);
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
        printf("Local group successfully created.\n");
        break;
    case ERROR_ALIAS_EXISTS:
        printf("Local group already exists.\n");
        err = 0;
        break;
    case ERROR_INVALID_PARAMETER:
        printf("Invalid parameter error adding local group; parameter index = %d\n",
            err, parm_err);
        NetApiBufferFree(lpszPrimaryDC);
        return(err);
    default:
        printf("Error adding local group: %d\n", err);
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
        printf("User successfully added to local group.\n");
        break;
    case ERROR_MEMBER_IN_ALIAS:
        printf("User already in local group.\n");
        err = 0;
        break;
    default:
        printf("Error adding user to local group: %d\n", err);
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
            printf("User successfully created.\n");
            break;
        case NERR_UserExists:
            printf("User already exists.\n");
            err = 0;
            break;
        case ERROR_INVALID_PARAMETER:
            printf("Invalid parameter error adding user; parameter index = %d\n",
                parm_err);
            return(err);
        default:
            printf("Error adding user: %d\n", err);
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
            printf("Local group successfully created.\n");
            break;
        case ERROR_ALIAS_EXISTS:
            printf("Local group already exists.\n");
            err = 0;
            break;
        case ERROR_INVALID_PARAMETER:
            printf("Invalid parameter error adding local group; parameter index = %d\n",
                err, parm_err);
            return(err);
        default:
            printf("Error adding local group: %d\n", err);
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
            printf("User successfully added to local group.\n");
            break;
        case ERROR_MEMBER_IN_ALIAS:
            printf("User already in local group.\n");
            err = 0;
            break;
        default:
            printf("Error adding user to local group: %d\n", err);
            break;
    }

    return(err);
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
