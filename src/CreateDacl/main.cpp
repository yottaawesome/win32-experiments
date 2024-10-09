// Adapted from https://learn.microsoft.com/en-us/windows/win32/secbp/creating-a-dacl
#include <windows.h>
#include <sddl.h>
import std;

#pragma comment(lib, "advapi32.lib")

// CreateMyDACL.
//    Create a security descriptor that contains the DACL 
//    you want.
//    This function uses SDDL to make Deny and Allow ACEs.
//
// Parameter:
//    SECURITY_ATTRIBUTES * pSA
//    Pointer to a SECURITY_ATTRIBUTES structure. It is your
//    responsibility to properly initialize the 
//    structure and to free the structure's 
//    lpSecurityDescriptor member when you have
//    finished using it. To free the structure's 
//    lpSecurityDescriptor member, call the 
//    LocalFree function.
// 
// Return value:
//    FALSE if the address to the structure is NULL. 
//    Otherwise, this function returns the value from the
//    ConvertStringSecurityDescriptorToSecurityDescriptor 
//    function.
bool CreateMyDACL(SECURITY_ATTRIBUTES* pSA)
{
    // Define the SDDL for the DACL. This example sets 
    // the following access:
    //     Built-in guests are denied all access.
    //     Anonymous logon is denied all access.
    //     Authenticated users are allowed 
    //     read/write/execute access.
    //     Administrators are allowed full control.
    // Modify these values as needed to generate the proper
    // DACL for your application. 

    // See https://learn.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format 
    // and https://learn.microsoft.com/en-us/windows/win32/secauthz/ace-strings
    // and https://learn.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-definition-language-for-conditional-aces-
    const wchar_t* szSD = L"D:"       // Discretionary ACL
        L"(D;OICI;GA;;;BG)"     // Deny access to 
        // built-in guests
        L"(D;OICI;GA;;;AN)"     // Deny access to 
        // anonymous logon
        L"(A;OICI;GRGWGX;;;AU)" // Allow 
        // read/write/execute 
        // to authenticated 
        // users
        L"(A;OICI;GA;;;BA)";    // Allow full control 
        // to administrators

    if (not pSA)
        return false;

    return ConvertStringSecurityDescriptorToSecurityDescriptor(
        szSD,
        SDDL_REVISION_1,
        &(pSA->lpSecurityDescriptor),
        nullptr
    );
}

int main()
{
    SECURITY_ATTRIBUTES  sa{
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .bInheritHandle = false
    };

    // Call function to set the DACL. The DACL
    // is set in the SECURITY_ATTRIBUTES 
    // lpSecurityDescriptor member.
    if (not CreateMyDACL(&sa))
    {
        // Error encountered; generate message and exit.
        std::println("Failed CreateMyDACL");
        std::exit(1);
    }

    // Use the updated SECURITY_ATTRIBUTES to specify
    // security attributes for securable objects.
    // This example uses security attributes during
    // creation of a new directory.
    if (not CreateDirectory(TEXT("C:\\MyFolder"), &sa))
    {
        // Error encountered; generate message and exit.
        std::println("Failed CreateDirectory");
        std::exit(1);
    }

    // Free the memory allocated for the SECURITY_DESCRIPTOR.
    if (LocalFree(sa.lpSecurityDescriptor))
    {
        // Error encountered; generate message and exit.
        std::println("Failed LocalFree");
        std::exit(1);
    }

    return 0;
}
