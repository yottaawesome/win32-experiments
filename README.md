# Win32 Experiments

## Introduction

This repo is a science lab. It's here for me to experiment with and test general Win32 concepts and API use in a quick and dirty fashion, without necessarily getting bogged down in the finer details.

## Building

You'll need Visual Studio 2022 Community Edition or above with the _Desktop development with C++_ workload installed (Visual Studio 2019 will also probably work if you downgrade the toolset). Some samples use [boost](https://www.boost.org/) and [node](https://nodejs.org/en/). You can either configure your environment to have boost (e.g. via [vcpkg](https://vcpkg.io/)) and node or remove these samples from the solution file if you don't want them.

## Considerations

The code is mostly adapted from existing MSDN samples or StackOverflow answers, with some minor changes and the occasional bugfix, as some MSDN samples do not even compile or have memory leaks. The code should not be assumed to be production quality, as program design, security and robustness have not been considered for these samples. The source code may suffer from bugs or other issues, such as lack of error checking, intermingling of C and C++ I/O, and security/deprecation problems. There may also be better ways of accomplishing the same functionality using other APIs or libraries. 

Some samples may use features only recently added in Windows, so no guarantee is given that all samples will run on all versions of Windows. Only x64 builds of the samples have been tested and run, the status and behaviour of x86 builds of the samples is unknown. No effort has been made to ensure the configuration settings between Debug and Release builds are appropriate or correct. Some samples may not even work correctly.

Whenever using the Win32 API, make sure to review the relevant MSDN API entry for relevant usage and security notes.

## Additional resources

* [Windows API Index](https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-api-list)
