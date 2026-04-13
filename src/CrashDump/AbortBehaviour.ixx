module;

#include <Windows.h>
#include <stdlib.h>
#include <cstdlib>

export module AbortBehaviour;

export namespace Abort::Sample1
{
	// Crash dump is generated in C:\Users\<userprofile>\AppData\Local\CrashDumps,
	// but only for release builds. In debug builds, the default behavior is to 
	// show a dialog instead of generating a dump.
	// See https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/set-abort-behavior?view=msvc-180
	void Run()
	{
		_set_abort_behavior(1, _CALL_REPORTFAULT);
		std::abort();
	}
}