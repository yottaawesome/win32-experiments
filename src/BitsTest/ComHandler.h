#pragma once
#include <Windows.h>
#include <memory>
#include "common.h"

class ComHandler final
{
	public:
		ComHandler(COINIT aparthmentThreadingMode = COINIT_MULTITHREADED);
		~ComHandler();
		
		template<typename T, typename U>
		std::shared_ptr<U> CreateInstance();
};

template<typename T, typename U>
std::shared_ptr<U> ComHandler::CreateInstance()
{
	U* pOut = nullptr;
	CheckHR(
		CoCreateInstance(
			__uuidof(T),
			nullptr,
			CLSCTX_LOCAL_SERVER,
			__uuidof(U),
			(void**)&pOut
		)
	);
	return std::shared_ptr<U>(pOut, [](U* ptr) -> void { ptr->Release(); });
}
