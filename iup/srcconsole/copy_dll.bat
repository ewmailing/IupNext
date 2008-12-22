@echo off  
copy /y ..\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\cd\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\im\lib\dll8\*.dll ..\bin\Win32
copy /y ..\..\lua5.1\lib\dll8\*.dll ..\bin\Win32
del ..\bin\Win32\*3.dll
