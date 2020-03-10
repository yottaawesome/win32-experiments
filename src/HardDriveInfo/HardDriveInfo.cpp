#include "Header.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")

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

// https://www.reddit.com/r/PowerShell/comments/8q4tbr/how_to_tell_ssd_from_hdd_via_cimwmi/e0ghf5i/
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/stormgmt/msft-physicaldisk?redirectedfrom=MSDN
// https://emoacht.wordpress.com/2012/11/06/csharp-ssd/
// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive?redirectedfrom=MSDN
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/1d4fda3c-885f-46e2-bc32-80c4426510dc/how-to-enumerate-all-disks-and-their-aggregated-volumes?forum=vcgeneral
int main(int argc, char** argv)
{
	ComInitialiser co;
	//return Win32Bios();
	//return Win32ComputerSystem();
	//return Win32OperatingSystem();
	//return Win32NetworkConnection();
	return Win32VideoController();
	//return MsftNetworkAdapter();
	//return Win32LogonSession();
	//return Win32ProcessorInfo();
	//return Win32RamInfo();
	//return Win32DiskInfo();
	//return MsftDiskInfo();
	//return Win32LogicalDisk();
}