#pragma comment(lib, "Comctl32.lib")

import std;
import Win32;
import ProgressBarSamples;

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	return ProgressBarSamples::App{}.Run();
}
