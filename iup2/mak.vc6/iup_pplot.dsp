# Microsoft Developer Studio Project File - Name="iup_pplot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iup_pplot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iup_pplot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iup_pplot.mak" CFG="iup_pplot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iup_pplot - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iup_pplot - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\include" /I "..\src" /I "..\srcpplot" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_CRT_SECURE_NO_DEPRECATE" /D "_IUP_PPLOT_" /D "CD_NO_OLD_INTERFACE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\include" /I "..\src" /I "..\srcpplot" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_CRT_SECURE_NO_DEPRECATE" /D "_IUP_PPLOT_" /D "CD_NO_OLD_INTERFACE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\$(ProjectName).lib" 
# ADD LIB32 /nologo /out:"..\lib\$(ProjectName).lib" 

!ENDIF

# Begin Target

# Name "iup_pplot - Win32 Debug"
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\iup_pplot.h
# End Source File
# Begin Source File

SOURCE=..\srcpplot\iup_pplot.hpp
# End Source File
# Begin Source File

SOURCE=..\srcpplot\PPlot.h
# End Source File
# Begin Source File

SOURCE=..\srcpplot\PPlotInteraction.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\srcpplot\iup_pplot.cpp
# End Source File
# Begin Source File

SOURCE=..\srcpplot\PPlot.cpp
# End Source File
# Begin Source File

SOURCE=..\srcpplot\PPlotInteraction.cpp
# End Source File
# End Target
# End Project

