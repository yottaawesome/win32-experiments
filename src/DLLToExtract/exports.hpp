#ifdef DO_EXTRACT_EXPORT
#define EXTERN extern "C" 
#define DLLEXPORT __declspec(dllexport)
#else
#define EXTERN
#define DLLEXPORT
#endif

EXTERN DLLEXPORT auto GetSecretOfTheUniverse() -> int;
