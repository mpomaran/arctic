# arctic

## Description

Software for low-power IoT sensors (like Arduino) with SIM900 module support, tested on Arduino Uno.

Features following:

- multitasking capability on bare atmega328/P processor (Arduino compatible)
- GSM support tested on SIM900
- simple encryption (beware - the current algorithm is quite simple to break, you may want to alter it - see ymessage.c)
- allows development and debugging under Windows (tested with VS 2017 community edition) and compilation for Arduino (CMake project)
- scripting (not used right now) based on uBasic

To build under Windows please open the VS2017 solution.
In order to build for Arduino please follow Arduino build - howto.txt.

Before the build you need to rename secrets.h.example to secrets.h and fill gateway url, incuding port (see [Arctic Panther](https://github.com/mpomaran/arctic)) and encryption key (must match the gateway). 

This project incorporates code from pt and uBasic projects, so their licenses apply.

## Development environment

The project compiles and runs on Windows as well as on Arduino. On Windows, the microcontroller’s hardware is emulated and most of the code uses API provided in the "arch" directory. Windows implementation uses the SIM900 emulator, where Arduino one needs to have the real device connected.

The provided ArcticPanther.SLN allows for development on Visual Studio Community Edition 2017.

Unit tests do not fit into Arduino memory, so they need to be enabled manually (look into tests.h file and uncomment necessary definitions).

## Building

### Windows

Open ArcticPanther.sln. Compile for x86 (64 bit is not tested).

If VS throws an error related to the SDK simply follow its advice and re-target.

### Cross compilation on Windows

1. Install arduino 1.0.5 - newer versions may not work correctly and were not tested (older versios might be obtained from <https://www.arduino.cc/en/main/OldSoftwareReleases>)
2. Install MinGW on c:\mingw
3. Run configure.bat, and:
    - Use "build" subdirectory as a place to build
    - Click "Configure"
    - Specify MinGW Make as a build system
    - Click "Specify toolchain file for cross-compiling", pick toolchain/ArduinoToolchain.cmake
4. Click configure && build
5. Execute make.bat

If you have any issues or errors during configuration please remove the cache re-run cmake-gui.
