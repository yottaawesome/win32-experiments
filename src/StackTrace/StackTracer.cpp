#include "StackTracer.h"

using namespace std;

#ifdef UNICODE
#define tstringstream wstringstream
#define tstring wstring
#define tcout wcout
#else
#define tstringstream stringstream
#define tstring string
#define tcout cout
#endif

// This function may be called at any point in time to get a print-out of the stack trace.
void PrintStackTrace()
{
	tstringstream stream;
	stream << TEXT("Started a stack trace") << std::endl;
	void* stack[TRACE_MAX_STACK_FRAMES];
	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	WORD numberOfFrames = CaptureStackBackTrace(0, TRACE_MAX_STACK_FRAMES, stack, NULL);
	SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
	if (symbol == nullptr)
		return;
	memset(symbol, 0, sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
	symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	DWORD displacement;
	IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
	if (line == nullptr)
		return;
	memset(line, 0, sizeof(IMAGEHLP_LINE64));
	line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	for (int i = 0; i < numberOfFrames; i++)
	{
		DWORD64 address = (DWORD64)(stack[i]);
		if (SymFromAddr(process, address, NULL, symbol))
		{
			if (SymGetLineFromAddr64(process, address, &displacement, line))
			{
				stream << TEXT("\tat ") << symbol->Name << TEXT(" in ") << line->FileName << TEXT(": line: ") << std::dec << line->LineNumber << TEXT(": address: ") << std::hex << symbol->Address << std::endl;
			}
			else
			{
				if (TRACE_LOG_ERRORS)
				{
					stream << TEXT("Error from SymGetLineFromAddr64: ") << std::hex << GetLastError() << std::endl;
				}
				stream << TEXT("\tat ") << symbol->Name << TEXT(": address: ") << std::hex << symbol->Address << std::endl;
			}
		}
		else if (TRACE_LOG_ERRORS)
		{
			stream << TEXT("Error from SymFromAddr: ") << std::hex << GetLastError() << std::endl;
		}
	}
	tstring str = stream.str();
	LPCTSTR message = str.c_str();
	std::tcout << message;
	free(symbol);
	free(line);
}

void PrintStackTraceFromExceptionContext(CONTEXT context, DWORD exceptionCode)
{
	tstringstream stream;
	if (exceptionCode)
	{
		stream << TEXT("Caught exception ") << std::hex << exceptionCode << std::endl;
	}
	else
	{
		stream << TEXT("Caught an exception") << std::endl;
	}
	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();
	// The image and frame will be set based on the processor architecture.
	// I don't set these addresses at random or through some voodoo;
	// they come from the documentation at http://msdn.microsoft.com/en-us/library/windows/desktop/ms680646(v=vs.85).aspx.
	DWORD image;
#ifdef _M_IX86
	STACKFRAME* frame = (STACKFRAME*)malloc(sizeof(STACKFRAME));
	memset(frame, 0, sizeof(STACKFRAME));
	image = IMAGE_FILE_MACHINE_I386;
	frame->AddrPC.Offset = context.Eip;
	frame->AddrPC.Mode = AddrModeFlat;
	frame->AddrFrame.Offset = context.Ebp;
	frame->AddrFrame.Mode = AddrModeFlat;
	frame->AddrStack.Offset = context.Esp;
	frame->AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	STACKFRAME64* frame = (STACKFRAME64*)malloc(sizeof(STACKFRAME64));
	memset(frame, 0, sizeof(STACKFRAME64));
	image = IMAGE_FILE_MACHINE_AMD64;
	frame->AddrPC.Offset = context.Rip;
	frame->AddrPC.Mode = AddrModeFlat;
	frame->AddrFrame.Offset = context.Rbp;
	frame->AddrFrame.Mode = AddrModeFlat;
	frame->AddrStack.Offset = context.Rsp;
	frame->AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	STACKFRAME64* frame = (STACKFRAME64*)malloc(sizeof(STACKFRAME64));
	memset(frame, 0, sizeof(STACKFRAME64));
	image = IMAGE_FILE_MACHINE_IA64;
	frame->AddrPC.Offset = context.StIIP;
	frame->AddrPC.Mode = AddrModeFlat;
	frame->AddrFrame.Offset = context.IntSp;
	frame->AddrFrame.Mode = AddrModeFlat;
	frame->AddrBStore.Offset = context.RsBSP;
	frame->AddrBStore.Mode = AddrModeFlat;
	frame->AddrStack.Offset = context.IntSp;
	frame->AddrStack.Mode = AddrModeFlat;
#else
#error "This platform is not supported."
#endif
	SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
	memset(symbol, 0, sizeof(SYMBOL_INFO) + (TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
	symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
	memset(line, 0, sizeof(IMAGEHLP_LINE64));
	line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD displacement = 0;
	while (StackWalk(image, process, thread, frame, &context, NULL, NULL, NULL, NULL) && frame->AddrPC.Offset != frame->AddrReturn.Offset && frame->AddrPC.Offset != 0)
	{
		if (SymFromAddr(process, frame->AddrPC.Offset, NULL, symbol))
		{
			if (SymGetLineFromAddr64(process, frame->AddrPC.Offset, &displacement, line))
			{
				stream << TEXT("\tat ") << symbol->Name << TEXT(" in ") << line->FileName << TEXT(": line: ") << std::dec << line->LineNumber << TEXT(": address: ") << std::hex << symbol->Address << std::endl;
			}
			else
			{
				if (TRACE_LOG_ERRORS)
				{
					stream << TEXT("Error from SymGetLineFromAddr64: ") << std::hex << GetLastError() << std::endl;
				}
				stream << TEXT("\tat ") << symbol->Name << TEXT(": address: ") << std::hex << symbol->Address << std::endl;
			}
		}
		else if (TRACE_LOG_ERRORS)
		{
			stream << TEXT("Error from SymFromAddr: ") << std::hex << GetLastError() << std::endl;
		}
	}
	DWORD error = GetLastError();
	if (error && TRACE_LOG_ERRORS)
	{
		stream << TEXT("Error from StackWalk64: ") << std::hex << error << std::endl;
	}
	tstring str = stream.str();
	LPCTSTR message = str.c_str();
	std::tcout << message;
	free(symbol);
	free(line);
	free(frame);
}

// This method is called on the tracer's constructor and tells Windows Error Reporting not only where to put crash dumps, but to actually generate them.
void SetRegistryKeyForWindowsErrorReportingToGenerateMiniCrashDumpsOnACrash()
{
	HKEY key;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting\\LocalDumps"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL) == ERROR_SUCCESS)
	{
		DWORD dumpCount = TRACE_DUMP_MAXIMUM_DUMP_FILES;
		RegSetValueEx(key, TEXT("DumpCount"), 0, REG_DWORD, (const BYTE*)(&dumpCount), sizeof(dumpCount));
		DWORD dumpType = TRACE_DUMP_TYPE;
		RegSetValueEx(key, TEXT("DumpType"), 0, REG_DWORD, (const BYTE*)(&dumpType), sizeof(dumpType));
		RegSetValueEx(key, TEXT("DumpFolder"), 0, REG_EXPAND_SZ, (const BYTE*)TEXT(TRACE_DUMP_FOLDER_LOCATION), sizeof(TEXT(TRACE_DUMP_FOLDER_LOCATION)));
		RegCloseKey(key);
	}
}

LONG WINAPI VectoredExceptionCallback(PEXCEPTION_POINTERS exception)
{
	if (exception != nullptr)
	{
		if (exception->ExceptionRecord->ExceptionCode && exception->ExceptionRecord->ExceptionCode <= 2147483647)
		{
			// This is a COM success code and can be ignored according to http://msdn.microsoft.com/en-us/library/windows/desktop/ff485841(v=vs.85).aspx.
			// Anything above that, and you'll want to log it out as an actual exception.
			tstringstream stream;
			stream 
				<< TEXT("Caught COM success code ") 
				<< std::hex << exception->ExceptionRecord->ExceptionCode 
				<< TEXT(". Ignoring it.") 
				<< std::endl;
			tstring str = stream.str();
			LPCTSTR message = str.c_str();
			std::tcout << message;
		}
		else
		{
			// Trace the stack from the exception (note, this may be a partial stack trace in scenarios where inline compiler optimizations are enabled).
			PrintStackTraceFromExceptionContext(*(exception->ContextRecord), exception->ExceptionRecord->ExceptionCode);
		}
	}
	// Send the exception back to the current process.
	return EXCEPTION_CONTINUE_SEARCH;
}

// This constructor should be called at the start of your main class constructor, and you should only ever create one instance of this per application.
StackTracer::StackTracer()
{
	if (ENABLE_WER_CRASH_DUMPS)
	{
		SetRegistryKeyForWindowsErrorReportingToGenerateMiniCrashDumpsOnACrash();
	}
	if (EXCEPTION_STACK_TRACING_ENABLED)
	{
		SymSetOptions(SYMOPT_DEFERRED_LOADS);
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		AddVectoredExceptionHandler(NULL, VectoredExceptionCallback);
	}
}

StackTracer::~StackTracer()
{
	if (EXCEPTION_STACK_TRACING_ENABLED)
	{
		SymCleanup(GetCurrentProcess());
	}
}