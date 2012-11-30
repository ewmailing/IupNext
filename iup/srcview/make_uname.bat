@echo off  

if "%1"==""     goto iupexe32
if "%1"=="vc11"  goto iupexe32
if "%1"=="vc11_64"  goto iupexe64
goto end

:iupexe32
call tecmake vc11 relink %2 %3 %4 %5 %6 %7
REM call tecmake vc11 "USE_GTK=Yes" relink %2 %3 %4 %5 %6 %7
goto end

:iupexe64
call tecmake vc11_64 relink %2 %3 %4 %5 %6 %7
REM call tecmake vc11_64 "USE_GTK=Yes" relink %2 %3 %4 %5 %6 %7
goto end

:end
