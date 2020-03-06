#include "Header.hpp"
#include <strsafe.h>
#include <sstream>

namespace Util
{
    std::string ConvertWStringToString(const std::wstring& wstr)
    {
        if (wstr.empty())
            return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    std::wstring ConvertStringToWString(const std::string& str)
    {
        if (str.empty())
            return std::wstring();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

        return wstrTo;
    }

    std::vector<std::wstring> TokeniseString(const std::wstring& stringToTokenise, const std::wstring& delimiter)
    {
        std::vector<std::wstring> results;
        size_t position = 0;
        std::wstring intermediateString = stringToTokenise;

        // If we don't find it at all, add the whole string
        if (stringToTokenise.find(delimiter, position) == std::string::npos)
        {
            results.push_back(stringToTokenise);
        }
        else
        {
            while ((position = intermediateString.find(delimiter, position)) != std::string::npos)
            {
                // split and add to the results
                std::wstring split = stringToTokenise.substr(0, position);
                results.push_back(split);

                // move up our position
                position += delimiter.length();
                intermediateString = stringToTokenise.substr(position);

                // On the last iteration, enter the remainder
                if (intermediateString.find(delimiter, position) == std::string::npos)
                    results.push_back(intermediateString);
            }
        }

        return results;
    }

    std::wstring Replace(std::wstring stringToWorkOn, const std::wstring& whatToReplace, const std::wstring& whatToReplaceWith)
    {
        std::vector<std::wstring> results = TokeniseString(stringToWorkOn, whatToReplace);
        if (results.size() == 0)
            return stringToWorkOn;

        std::wstring updatedString = L"";
        for (int i = 0; i < results.size() - 1; i++)
        {
            updatedString += results.at(i);
            updatedString += whatToReplaceWith;
        }
        updatedString += results.at(results.size() - 1);

        return updatedString;
    }

    void PrintLastError(wchar_t* lpszFunction)
    {
        // Retrieve the system error message for the last-error code

        LPVOID lpMsgBuf;
        LPVOID lpDisplayBuf;
        DWORD dw = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (LPTSTR)&lpMsgBuf,
            0,
            nullptr);

        lpDisplayBuf = (LPVOID)LocalAlloc(
            LMEM_ZEROINIT,
            (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        StringCchPrintf((LPTSTR)lpDisplayBuf,
            LocalSize(lpDisplayBuf) / sizeof(TCHAR),
            TEXT("%s failed with error %d: %s"),
            lpszFunction,
            dw,
            lpMsgBuf);

        // TO DO log the error

        LocalFree(lpMsgBuf);
        LocalFree(lpDisplayBuf);
    }

    void CheckHr(HRESULT hr, const std::string& msg)
    {
        if (FAILED(hr))
        {
            std::stringstream ss;
            _com_error ce(hr);
            ss
                << msg
                << std::endl
                << " Error code: 0x" << std::hex << hr
                << std::endl
                << " COM error message: " << Util::ConvertWStringToString(ce.ErrorMessage())
                << std::endl;
            throw std::runtime_error(ss.str());
        }
    }

    void CheckHr(HRESULT hr, ErrorCallback lambda)
    {
        if (FAILED(hr))
        {
            std::stringstream ss;
            _com_error ce(hr);
            ss
                << lambda()
                << std::endl
                << " Error code: 0x" << std::hex << hr
                << std::endl
                << " COM error message: " << ce.ErrorMessage()
                << std::endl;
            throw std::runtime_error(ss.str());
        }
    }
}