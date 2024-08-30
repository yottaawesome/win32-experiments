import std;

#ifdef EXPLICIT_EXPORTS
#define EXPORT __declspec(dllexport)
#define EXPORT_CLASS __declspec(dllexport)
#define EXTERN_EXPORT extern "C" __declspec(dllexport) 
#else
#define EXPORT 
#define EXPORT_CLASS __declspec(dllimport)
#define EXTERN_EXPORT
#endif

namespace ExplicitDLL
{
	extern "C" EXPORT void Something();

	struct ISomeObject
	{
		virtual ~ISomeObject() = default;
		virtual void DoIt() = 0;
	};

	extern "C" EXPORT ISomeObject* Make();
}