#include "ComHandler.h"
#include "common.h"

ComHandler::ComHandler(COINIT apartmentThreadingModel)
{
    CheckHR(CoInitializeEx(nullptr, apartmentThreadingModel));
}

ComHandler::~ComHandler()
{
    CoUninitialize();
}
