#include "Header.hpp"

uint64_t FromFileTime(const FILETIME& ft) {
	ULARGE_INTEGER uli = { 0 };
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	return uli.QuadPart;
}

int GetSystemTimesTest()
{
	uint64_t idleTimeP = 0;
	uint64_t kernelTimeP = 0;
	uint64_t userTimeP = 0;

	for (int i = 0; i < 10; i++)
	{
		FILETIME idleTime1;
		FILETIME kernelTime1;
		FILETIME userTime1;

		GetSystemTimes(&idleTime1, &kernelTime1, &userTime1);

		uint64_t idleTimeC = FromFileTime(idleTime1);
		uint64_t kernelTimeC = FromFileTime(kernelTime1);
		uint64_t userTimeC = FromFileTime(userTime1);

		if (idleTimeP != 0)
		{
			uint64_t idleDiff = idleTimeC - idleTimeP;
			uint64_t trueKernelTime = kernelTimeC - kernelTimeP - idleDiff;
			uint64_t userDiff = userTimeC - userTimeP;

			float total = idleDiff + trueKernelTime + userDiff;

			float idlePercent = idleDiff / total * 100.0f;
			float kernelTimePercent = trueKernelTime / total * 100.0f;
			float userTimePercent = userDiff / total * 100.0f;

			std::wcout << "Total: " << idlePercent + kernelTimePercent + userTimePercent << std::endl;
			std::wcout << "Idle: " << idleDiff / total * 100.0f << std::endl;
			std::wcout << "Kernel: " << trueKernelTime / total * 100.0f << std::endl;
			std::wcout << "User: " << userDiff / total * 100.0f << std::endl;
		}

		idleTimeP = idleTimeC;
		kernelTimeP = kernelTimeC;
		userTimeP = userTimeC;

		Sleep(1000);
	}

	

	return 0;
}