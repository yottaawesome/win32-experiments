#pragma once
#include <Windows.h>

class ComInitializer final
{
	public:
		ComInitializer(COINIT aparthmentThreadingMode = COINIT_MULTITHREADED);
		~ComInitializer();
};
