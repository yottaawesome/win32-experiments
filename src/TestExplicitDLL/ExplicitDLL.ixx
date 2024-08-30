module;

#define EXPLICIT_EXPORTS
#include "ExplicitDLL.hpp"

export module explicitdll;

namespace ExplicitDLL
{
	extern "C" EXPORT void Something()
	{
	}
}