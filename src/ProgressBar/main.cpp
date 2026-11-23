#pragma comment(lib, "Comctl32.lib")

import std;
import Win32;
import MultiControlSamples;

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	return MultiControlSamples::App{}.Run();
}
