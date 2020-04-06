#include "Header.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")
#include <stack>

using RequestStackState = std::tuple<DWORD, std::wstring>;

class WinHttpRequestStatus
{
	public:
		void Begin(std::wstring path);
		void PushState(RequestStackState& state);
		RequestStackState GetLast();
		RequestStackState Complete();

	private:
		std::stack<RequestStackState> m_state;
};

void WinHttpRequestStatus::Begin(std::wstring path)
{
	while (!m_state.empty())
		m_state.pop();
	m_state.push(RequestStackState(0, path));
};

void WinHttpRequestStatus::PushState(RequestStackState& state)
{
	m_state.push(state);
};

RequestStackState WinHttpRequestStatus::Complete()
{
	return m_state.top();
}

void Blah()
{
	WinHttpRequestStatus ww;
	RequestStackState ss(200, L"");
	ww.PushState(ss);
}

// https://www.reddit.com/r/PowerShell/comments/8q4tbr/how_to_tell_ssd_from_hdd_via_cimwmi/e0ghf5i/
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/stormgmt/msft-physicaldisk?redirectedfrom=MSDN
// https://emoacht.wordpress.com/2012/11/06/csharp-ssd/
// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive?redirectedfrom=MSDN
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/1d4fda3c-885f-46e2-bc32-80c4426510dc/how-to-enumerate-all-disks-and-their-aggregated-volumes?forum=vcgeneral
int main(int argc, char** argv)
{
	for (int i = 0; i < 5; i++)
		Blah();
	return 0;

	ComInitialiser co;

	//return Win32PerfRawDataPerfDiskLogicalDisk();
	//return Win32LogicalDisk();
	return MSStorageDriverFailurePredictStatus();
	//return GetSystemTimesTest();
	//return Win32PerfRawDataPerfProcProcess();
	//return Win32DiskPartition();
	//return Win32Bios();
	//return Win32ComputerSystem();
	//return Win32OperatingSystem();
	//return Win32NetworkConnection();
	//return Win32VideoController();
	//return MsftNetworkAdapter();
	//return Win32LogonSession();
	//return Win32LoggedOnUser();
	//return Win32ProcessorInfo();
	//return Win32RamInfo();
	//return Win32DiskInfo();
	//return MsftDiskInfo();
	//return Win32LogicalDisk();
}