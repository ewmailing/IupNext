# Microsoft Developer Studio Project File - Name="iupmot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iupmot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupmot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupmot.mak" CFG="iupmot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupmot - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupmot - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\src\win" /I "..\include" /I "d:\LNG\x11inc" /I "D:\_goodies\iup\_libs\openmotif2.1.30\lib" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WIN32_WINNT=0x0400" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\src\win" /I "..\include" /I "d:\LNG\x11inc" /I "D:\_goodies\iup\_libs\openmotif2.1.30\lib" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WIN32_WINNT=0x0400" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
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

# Name "iupmot - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\src\mot\Canvas.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motcolor.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motcreat.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motfiledlg.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motget.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motglobal.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\mothelp.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motif.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motkey.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motmsgdlg.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motproc.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motresiz.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\motset.c

!IF  "$(CFG)" == "iupmot - Win32 Debug"

# ADD CPP /nologo /I "C:\APP\LNG\x11inc" /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\mot\motsize.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\mottimer.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\mottips.c
# End Source File
# Begin Source File

SOURCE=..\src\mot\utilsx11.c
# End Source File
# End Group
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\mot\Canvas.h

!IF  "$(CFG)" == "iupmot - Win32 Debug"

# PROP Ignore_Default_Tool 1
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\mot\CanvasP.h

!IF  "$(CFG)" == "iupmot - Win32 Debug"

# PROP Ignore_Default_Tool 1
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\mot\motif.h

!IF  "$(CFG)" == "iupmot - Win32 Debug"

# PROP Ignore_Default_Tool 1
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\mot\mottips.h

!IF  "$(CFG)" == "iupmot - Win32 Debug"

# PROP Ignore_Default_Tool 1
!ENDIF

# End Source File
# End Group
# End Target
# End Project

