# Win32 Experiments

## Introduction

This repo is to experiment with Win32 code.

## Building

You'll need Visual Studio 2022 Community Edition or above with the _Desktop development with C++_ workload installed. Some samples use [boost](https://www.boost.org/) and [node](https://nodejs.org/en/). You can either configure your environment to have boost (e.g. via [vcpkg](https://vcpkg.io/)) and node or remove these samples from the solution file if you don't want them.

## Considerations

The code is mostly adapted from existing MSDN samples or StackOverflow answers, with some minor changes and the occasional bugfix, as some MSDN samples do not even compile or have memory leaks. As such, it should not be assumed to be production quality and may suffer from bugs or other issues, such as lack of error checking, intermingling of C and C++ IO, and security/deprecation problems. There may also be better ways of accomplishing the same functionality using other APIs or libraries. This repo is merely intended to demonstrate and test general Win32 concepts and API use in a quick and dirty fashion, without necessarily getting bogged down in the details (although some samples may be more elaborate then others). Security and robustness has not been considered for these samples, so make sure to review the relevant MSDN API entry for any hidden gotchas.

Some samples may use features only recently added in Windows, so no guarantee is given that all samples will run on all versions of Windows. Only x64 builds of the samples have been tested and run, the status and behaviour of x86 builds of the samples is unknown. No effort has been made to ensure the configuration settings between Debug and Release builds are appropriate or correct.

## Additional resources

* [Windows API Index](https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-api-list)
