@echo off
REM This builds all the libraries for 1 uname

FOR %%u IN (src srccd srccontrols srcmatrixex srcgl srcglcontrols srcplot srcpplot srcmglplot srcscintilla srcim srcimglib srcole srctuio srcweb srcledc srcview srclua5 srcconsole test) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9

