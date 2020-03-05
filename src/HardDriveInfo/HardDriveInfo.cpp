#include "Header.hpp"
#include <vector>
#include <string>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")

std::vector<std::wstring> TokeniseString(std::wstring stringToTokenise, std::wstring delimiter)
{
    std::vector<std::wstring> results;
    size_t position = 0;
	std::wstring intermediateString = stringToTokenise;

	// If we don't find it at all, add the whole string
	if (stringToTokenise.find(delimiter, position) == std::string::npos)
		results.push_back(stringToTokenise);

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

    return results;
}

// https://www.reddit.com/r/PowerShell/comments/8q4tbr/how_to_tell_ssd_from_hdd_via_cimwmi/e0ghf5i/
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/stormgmt/msft-physicaldisk?redirectedfrom=MSDN
// https://emoacht.wordpress.com/2012/11/06/csharp-ssd/
// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive?redirectedfrom=MSDN
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/1d4fda3c-885f-46e2-bc32-80c4426510dc/how-to-enumerate-all-disks-and-their-aggregated-volumes?forum=vcgeneral
int main(int argc, char** argv)
{

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	//return Win32ProcessorInfo();
	//return Win32RamInfo2();
	//return Win32RamInfo();
	return Win32DiskInfo();
	//return MsftDiskInfo();
	//return Win32LogicalDisk();
}