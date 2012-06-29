@echo off  
mkdir ..\bin\Win32\Lua5.1
copy /y ..\lib\dll8\*.dll ..\bin\Win32\Lua5.1
copy /y ..\..\cd\lib\dll8\*.dll ..\bin\Win32\Lua5.1
copy /y ..\..\im\lib\dll8\*.dll ..\bin\Win32\Lua5.1
copy /y ..\..\lua5.1\lib\dll8\*.dll ..\bin\Win32\Lua5.1
copy /y ..\..\luagl\lib\dll8\*.dll ..\bin\Win32\Lua5.1
copy /y ..\..\lfs\lib5.1\dll8\*.dll ..\bin\Win32\Lua5.1
mkdir ..\bin\Win32\Lua5.1\Microsoft.VC80.CRT
copy /y ..\..\lua5.1\bin\Win32\Microsoft.VC80.CRT ..\bin\Win32\Lua5.1\Microsoft.VC80.CRT\
del ..\bin\Win32\Lua5.1\*3.dll
del ..\bin\Win32\Lua5.1\*52.dll
move /y ..\bin\Win32\iuplua51.exe ..\bin\Win32\Lua5.1\

mkdir ..\bin\Win32\Lua52
copy /y ..\lib\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\..\cd\lib\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\..\im\lib\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\..\lua52\lib\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\..\luagl\lib\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\..\lfs\lib52\dll10\*.dll ..\bin\Win32\Lua52
copy /y ..\dist\Microsoft.VC100.CRT\dll10\* ..\bin\Win32\Lua52
del ..\bin\Win32\Lua52\*3.dll
del ..\bin\Win32\Lua52\*51.dll
move /y ..\bin\Win32\iuplua52.exe ..\bin\Win32\Lua52\

REM This actually fails for a few files, 
REM because del is removing more files than expected.
