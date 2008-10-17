# Microsoft Developer Studio Project File - Name="iupluacontrols5" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iupluacontrols5 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupluacontrols5.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupluacontrols5.mak" CFG="iupluacontrols5 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupluacontrols5 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupluacontrols5 - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1046 
# ADD RSC /l 1046 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\iupluacontrols51.lib" 
# ADD LIB32 /nologo /out:"..\lib\iupluacontrols51.lib" 

!ENDIF

# Begin Target

# Name "iupluacontrols5 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\srclua5\cbox.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\cells.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\colorbar.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\colorbrowser.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\controls.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\dial.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\gauge.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\gc.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\getparam.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\mask.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\matrix.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\matrixfuncs.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\sbox.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\spin.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\spinbox.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\tabs.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\tree.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\treefuncs.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\val.c
# End Source File
# End Group
# Begin Group "Lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\cbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\cells.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\colorbar.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\colorbrowser.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\dial.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\gauge.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\matrix.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\sbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\spin.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\spinbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\tabs.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\tree.lua
# End Source File
# Begin Source File

SOURCE=..\srclua5\val.lua
# End Source File
# End Group
# Begin Group "Includes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\il_controls.h
# End Source File
# Begin Source File

SOURCE=..\include\iupluacontrols.h
# End Source File
# Begin Source File

SOURCE=..\srclua5\luagetparam.h
# End Source File
# Begin Source File

SOURCE=..\srclua5\luamask.h
# End Source File
# End Group
# End Target
# End Project

