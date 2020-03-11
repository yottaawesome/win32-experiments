#include "Header.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")

// https://www.reddit.com/r/PowerShell/comments/8q4tbr/how_to_tell_ssd_from_hdd_via_cimwmi/e0ghf5i/
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/stormgmt/msft-physicaldisk?redirectedfrom=MSDN
// https://emoacht.wordpress.com/2012/11/06/csharp-ssd/
// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive?redirectedfrom=MSDN
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/1d4fda3c-885f-46e2-bc32-80c4426510dc/how-to-enumerate-all-disks-and-their-aggregated-volumes?forum=vcgeneral
int main(int argc, char** argv)
{
	ComInitialiser co;

	//return Win32PerfRawDataPerfProcProcess();
	//return Win32DiskPartition();
	//return Win32Bios();
	//return Win32ComputerSystem();
	//return Win32OperatingSystem();
	//return Win32NetworkConnection();
	return Win32VideoController();
	//return MsftNetworkAdapter();
	//return Win32LogonSession();
	//return Win32LoggedOnUser();
	//return Win32ProcessorInfo();
	//return Win32RamInfo();
	//return Win32DiskInfo();
	//return MsftDiskInfo();
	//return Win32LogicalDisk();
}