@echo off  

if "%1"==""     goto iupexe32
if "%1"=="vc8"  goto iupexe32
if "%1"=="vc8_64"  goto iupexe64
if "%1"=="all"  goto iupexe
if "%1"=="copy-dll"     goto copy-dll
if "%1"=="copy-dll64"     goto copy-dll64
goto end

:copy-dll
copy /y ..\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\cd\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\im\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\lua5.1\lib\dll8\*.dll ..\bin\Win32
del ..\bin\Win32\*3.dll
goto end

:copy-dll64
copy /y ..\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\cd\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\im\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\lua5.1\lib\dll8_64\*.dll ..\bin\Win64
del ..\bin\Win64\*3.dll
goto end

:iupexe32
call tecmake dll8 relink %2 %3 %4 %5 %6 %7
REM call tecmake dll8 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:iupexe64
call tecmake dll8_64 relink %2 %3 %4 %5 %6 %7
REM call tecmake dll8_64 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8_64 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:iupexe
call tecmake dll8 relink %2 %3 %4 %5 %6 %7
REM call tecmake dll8 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
call tecmake dll8_64 relink %2 %3 %4 %5 %6 %7
REM call tecmake dll8_64 USE_GTK=Yes relink %2 %3 %4 %5 %6 %7
REM call tecmake vc8_64 "MF=iuplua3" relink %2 %3 %4 %5 %6 %7
goto end

:end
