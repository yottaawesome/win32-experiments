#pragma once
// From https://www.dima.to/blog/?p=13

/*
StackTracer - v1.0 by Alexandru Dima

By default, StackTracer enables WER (Windows Error Reporting) to create crash dumps. This can be toggled by setting ENABLE_WER_CRASH_DUMPS to TRUE or FALSE. What it does is it creates a registry
key that tells Windows Error Reporting to generate crash dumps whenever an application crashes. The location of the dumps depend on what account you run the application with. They can can be dumped
out at C:\Windows\System32\TRACE_DUMP_FOLDER_LOCATION for example if running under the LOCAL SYSTEM account in Windows, or the respective user account folder for your current user account, just as an
example. WER will then try dumping all application crashes on the current machine there, not just for the application you run this code on. These dumps (.dmp files) located at TRACE_DUMP_FOLDER_LOCATION
can be opened in Visual Studio and used, against matching code (meaning your binaries have to match your code), to debug into the crash location. Therefore, it is recommended that you perfectly
save the code used in a particular deployment.

You can set EXCEPTION_STACK_TRACING_ENABLED to TRUE if you wish to capture stack traces on unhandled exceptions, as they will go to the vectored exception handler and print out the stack for that
exception. By default this feature is enabled (TRUE). Once enabled, you might see partial stack traces. This is because the inline optimization is enabled, or because your method is explicitly marked
as inline. You can disable this feature on your project, by right-clicking it and navigating to Properties -> Configuration Properties -> C++ -> Optimization -> Inline Function Expansion and
setting it to Disabled (/Ob0). For inline methods, you should search your project for the inline keyword and remove these methods temporarily.

Also, if you ever need to print a complete stack trace from somewhere in your code, you may use the PrintStackTrace() method. This will help if you have multiple threads contending for the
same method, and are trying to figure out why something of that nature isn't working. Inline methods and the inline optimizations may also cause you to see partial stack traces with this
method as well, so it is recommended that you disable it while testing.

And as always, in order for everything to work with stack tracing, the PDB files must be present. Please ensure that your project is set to output debug information and has its PDBs in the
working directory. You may set this by right-clicking your project, navigating to Properties -> Configuration Properties -> Linker -> Debugging, and set Generate Debug Info to Yes (/DEBUG),
also making sure to set GenerateProgramDatabaseFile right below it to your output directory (typically, this is $(OutDir)$(TargetName).pdb).

Once enabled, StackTracer's methods output to the standard output. Note, you must include dbghelp.lib in the Linker's dependencies in order to make use of these API's in another project.
*/

#include <Windows.h>
#include <dbghelp.h>
#include <sstream>
#include <iostream>

// Enables or disables stack tracing.
#define ENABLE_WER_CRASH_DUMPS TRUE
// Enables or disables stack tracing.
#define EXCEPTION_STACK_TRACING_ENABLED TRUE
// Sets the maximum function name of all functions when doing stack tracing.
#define TRACE_MAX_FUNCTION_NAME_LENGTH 1024
// Sets the maximum number of stack frames to walk when calling PrintStackTrace().
#define TRACE_MAX_STACK_FRAMES 1024
// Determines wether or not to catch exceptions as part of the WINAPI stack tracing methods.
#define TRACE_LOG_ERRORS FALSE
// This setting tells Windows Error Reporting to dump crashes to C:\Windows\System32\TRACE_DUMP_FOLDER_LOCATION under Local System credentials (services)...once set, it tries to dump ALL application crashes.
#define TRACE_DUMP_FOLDER_LOCATION "CrashDumps"
// Sets the maximum number of dump files for your TRACE_DUMP_FOLDER_LOCATION. Once max'd out, it will roll over.
#define TRACE_DUMP_MAXIMUM_DUMP_FILES 100
// Sets the dump type to a mini-dump. Possible values are: 0 (Custom Dump), 1 (Mini Dump), or 2 (Full Dump).
#define TRACE_DUMP_TYPE 1

void PrintStackTrace();

class StackTracer
{
public:
	StackTracer();
	~StackTracer();
};