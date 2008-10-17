@echo off
REM This builds all the libraries of the folder for 1 uname

if "%1"=="vc7" goto build
if "%1"=="dll7" goto build
if "%1"=="all" goto build_all
goto fim

:build
call tecmake %1 %2 %3 %4 %5 %6
goto fim

:build_all
call tecmake vc7 %2 %3 %4 %5 %6
call tecmake dll7 %2 %3 %4 %5 %6
goto fim

:fim
