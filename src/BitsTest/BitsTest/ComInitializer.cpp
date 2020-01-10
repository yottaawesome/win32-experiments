#include "ComInitializer.h"
#include "common.h"

ComInitializer::ComInitializer(COINIT aparthmentThreadingMode)
{
    CheckHR(CoInitializeEx(nullptr, aparthmentThreadingMode));
}

ComInitializer::~ComInitializer()
{
    CoUninitialize();
}
