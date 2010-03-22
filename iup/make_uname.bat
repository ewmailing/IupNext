@echo off
REM This builds all the libraries for 1 uname
                                  
FOR %%u IN (src srccd srccontrols srcpplot srcgl srcim srcole srcledc srcview srclua3 srclua5 srcconsole) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9                                  
REM FOR %%u IN (src srccd srccontrols srcpplot srcgl srcim srcimglib srcole srcledc srcview srclua5 srcconsole) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9
