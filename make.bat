cd build
c:\MinGW\bin\mingw32-make.exe clean
if %errorlevel% neq 0 goto ERROR
c:\MinGW\bin\mingw32-make.exe VERBOSE=1 
if %errorlevel% neq 0 goto ERROR
goto END
:ERROR
pause
cd ..
exit /b %errorlevel%
:END
pause
cd ..
