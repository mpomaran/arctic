cd build
c:\MinGW\bin\mingw32-make clean
if %errorlevel% neq 0 GOTO ERROR
c:\MinGW\bin\mingw32-make
if %errorlevel% neq 0 GOTO ERROR
cd ArcticPanther
"C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\avrdude.exe" "-CC:/Program Files (x86)/Arduino/hardware/tools/avr/etc/avrdude.conf" -v -v -v -v -patmega328p -cusbasp -Pusb -Uflash:w:arctic_panther.hex
goto OK
:ERROR
cd ..
pause
exit /b %errorlevel%
:OK
pause
cd ../..
