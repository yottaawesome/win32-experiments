# Win32 Experiments

## Introduction

This repo is just to test out some experimental Win32 code.

## Considerations

The code is mostly adapted from existing MSDN samples or StackOverflow answers, with some minor changes and the occasional bugfix, as some MSDN samples do not even compile or have memory leaks. As such, it's not production quality and may suffer from bugs or other issues, such as lack of error checking, intermingling of C and C++ IO, and security/deprecation problems. There may also be better ways of accomplishing the same functionality using other APIs or libraries. This repo is merely intended to demonstrate and test general Win32 concepts and API use in a quick and dirty fashion, without necessarily getting bogged down in the details (although some samples may be more elaborate then others). Security and robustness has not been considered for these samples, so make sure to review the relevant MSDN API entry for any hidden gotchas.

Some samples use [boost](https://www.boost.org/) and [node](https://nodejs.org/en/). You can either configure your environment to have boost (e.g. via [vcpkg](https://vcpkg.io/)) and node or remove these samples from the solution file if you don't want them.

## Additional resources

* [Windows API Index](https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-api-list)
