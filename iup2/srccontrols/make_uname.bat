@echo off
REM This builds all the libraries of the folder for 1 uname

call tecmake %1 %2 %3 %4 %5 %6
call tecmake %1 "MF=iupimglib" %2 %3 %4 %5 %6
