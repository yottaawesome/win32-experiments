#define DO_EXTRACT_EXPORT
#include "exports.hpp"

extern "C"
{
	int __declspec(dllexport) GetSecretOfTheUniverse()
	{
		return 42;
	}
}
