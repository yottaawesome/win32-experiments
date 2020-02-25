#pragma once
#include <Windows.h>
#include <memory>
#include "common.h"

class ComHandler final
{
	public:
		ComHandler(COINIT aparthmentThreadingMode = COINIT_MULTITHREADED);
		~ComHandler();
		
		template<typename I, typename C>
		std::shared_ptr<I> CreateInstance();
};

template<typename I, typename C>
std::shared_ptr<I> ComHandler::CreateInstance()
{
	I* pOut = nullptr;
	CheckHR(
		CoCreateInstance(
			__uuidof(C),
			nullptr,
			CLSCTX_LOCAL_SERVER,
			__uuidof(I),
			(void**)&pOut
		)
	);
	return std::shared_ptr<I>(pOut, [](I* ptr) -> void { if(ptr != nullptr) ptr->Release(); });
}
