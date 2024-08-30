#ifdef EXPLICIT_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

namespace ExplicitDLL
{
	extern "C" EXPORT void Something();
}