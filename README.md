# arctic

Software for low power IoT sensors with SIM900 module support. It's fairly portable, but was tested on Arduino Uno only.

The counterpart project - the server for M2M applications is coming, but not ready to be released yet.

Features following:
 - multitasking capability on bare atmega328/P processor (arduino compatible)
 - GSM support tested on SIM900
 - simple encryption (beware - current algorithm is quite simple to break, you may want to alter it - see ymessage.c)
 - allows development and debugging under Windows (tested with VS 2017 community edition) and compilation for Arduino (cmake project)
 - scripting (not used right now) based on uBasic
 
To build under Windows please open the VS2017 solution.
In order to build for Arduino please follow Arduino build - howto.txt.

Please set correct URL in ArcticPanther.cpp (M2M_URL) and encryption key in ymessage.c (SECRET_KEY)

This project incorporates code from pt and ubasic projects, so their licenses apply when needed.