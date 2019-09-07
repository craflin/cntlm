# CNTLM

This is an untested work in progress CMake and Win32 port of the original cntlm daemon by David Kubicek (http://cntlm.sourceforge.net/) with a slightly reduced feature set (no ISA scanner, removed legacy code). The project is mostly educational, I'm not trying to continue development of CNTLM or to maintain a modernized code base.

## Build Instructions

Just build the project with CMake using GCC (on Linux) or some version of Visual Studio (on Windows). A Debian DEB package can be created using the `package` target.
