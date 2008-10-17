# Microsoft Developer Studio Project File - Name="iuplua5" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iuplua5 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iuplua5.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iuplua5.mak" CFG="iuplua5 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iuplua5 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\lib"
# PROP BASE Intermediate_Dir "..\obj\$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\obj\$(ProjectName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1046 
# ADD RSC /l 1046 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\iuplua51.lib" 
# ADD LIB32 /nologo /out:"..\lib\iuplua51.lib" 

!ENDIF

# Begin Target

# Name "iuplua5 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\srclua5\iuplua.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\iuplua_api.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\scanf.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\button.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\canvas.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\dialog.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\filedlg.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\fill.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\frame.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\hbox.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\image.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\item.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\label.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\list.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\menu.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\multiline.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\radio.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\separator.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\submenu.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\text.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\timer.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\toggle.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\vbox.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\srclua5\zbox.c

!IF  "$(CFG)" == "iuplua5 - Win32 Debug"

# ADD CPP /nologo /W4 /GZ /GX 
!ENDIF

# End Source File
# End Group
# End Group
# Begin Group "Lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\constants.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\ctrl_template.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\generator.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\iuplua.lua
# End Source File
# Begin Group "controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\button.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\canvas.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\colordlg.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\dialog.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\filedlg.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\fill.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\fontdlg.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\frame.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\hbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\image.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\imagergb.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\imagergba.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\item.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\label.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\list.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\menu.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\messagedlg.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\multiline.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\radio.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\separator.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\submenu.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\text.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\timer.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\toggle.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\vbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\zbox.lua
# End Source File
# End Group
# End Group
# Begin Group "Includes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\il.h
# End Source File
# Begin Source File

SOURCE=..\include\iuplua.h
# End Source File
# Begin Source File

SOURCE=..\srclua5\scanf.h
# End Source File
# End Group
# End Target
# End Project

