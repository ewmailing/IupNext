@echo off  
copy /y ..\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\cd\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\im\lib\dll8_64\*.dll ..\bin\Win64
copy /y ..\..\lua5.1\lib\dll8_64\*.dll ..\bin\Win64
del ..\bin\Win64\*3.dll
