module;

#define EXPLICIT_EXPORTS
#include "ExplicitDLL.hpp"

export module explicitdll;

export namespace ExplicitDLL
{
	extern "C" EXPORT void Something()
	{
	}

	struct SomeObject : public ISomeObject
	{
		void DoIt() override {}
	};

	extern "C" EXPORT ISomeObject* Make()
	{
		return new SomeObject();
	}
}