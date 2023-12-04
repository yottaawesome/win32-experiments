# Basic COM

## Introduction

This repo is an implementation of an old [CodeGuru tutorial on authoring a COM DLL](https://www.codeguru.com/soap/step-by-step-com-tutorial/). The links and images are broken on the original page (Dr Dobbs itself sadly ceased publication a decade ago), but you can find an upload of the original source code [here](https://github.com/grimripper/grim_com_testing/tree/master/COMStepByStep_src).

## Changes

The sample has been updated to fix multiple bugs when registering the DLL in the registry. The two relevant keys it creates are `HKEY_CLASSES_ROOT\CLSID\{92E7A9C2-F4CB-11D4-825D-00104B3646C0}` and `HKEY_CLASSES_ROOT\CodeGuru.FastAddition`.

## Building

You need Visual Studio 2022 with the desktop development with C++ workload. You can then open the solution (or just the project) and build it.

## Running

You first need to register the DLL after it's built so COM knows how to instantiate the class. Fortunately, the sample supports self-installation: from an administrative cmd prompt, navigate to where the build DLL is and run `regsvr32 basic-com.dll`. To unregister it, do the same but run `regsvr32 /u basic-com.dll`. You can then run the associated `basic-com-client` project (note that it runs under the apartment-threaded concurrency model -- if you change the concurrency model to multithreaded, it will fail with `E_NOINTERFACE`).

## Additional resources

* [Can't use __declspec(dllexport)](https://stackoverflow.com/questions/3460533/why-cant-i-use-declspecdllexport-to-export-dllgetclassobject-from-a-com-d)
* [Module Definition Files (DEFs)](https://learn.microsoft.com/en-us/cpp/build/reference/module-definition-dot-def-files?view=msvc-170)
* [Exporting from a DLL with DEF files](https://learn.microsoft.com/en-us/cpp/build/exporting-from-a-dll-using-def-files?view=msvc-170)
* [Registering COM applications](https://learn.microsoft.com/en-us/windows/win32/com/registering-com-applications)
