# Microsoft Developer Studio Project File - Name="iup" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iup - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iup.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iup.mak" CFG="iup - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iup - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iup - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\src\win" /I "..\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WIN32_WINNT=0x0500" /D "_WIN32_IE=0x0500" /D "WINVER=0x0500" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\src\win" /I "..\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_WIN32_WINNT=0x0500" /D "_WIN32_IE=0x0500" /D "WINVER=0x0500" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
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

# Name "iup - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Group "win"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\win\win.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winbutton.c
# End Source File
# Begin Source File

SOURCE=..\src\win\wincombo.c
# End Source File
# Begin Source File

SOURCE=..\src\win\wincreat.c
# End Source File
# Begin Source File

SOURCE=..\src\win\windestroy.c
# End Source File
# Begin Source File

SOURCE=..\src\win\windlg.c

!IF  "$(CFG)" == "iup - Win32 Debug"

# ADD CPP /nologo /W4 /TC /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\win\windraw.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winedit.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winfiledlg.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winframe.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winget.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winglobal.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winhandle.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winhelp.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winhook.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winicon.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winkey.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winmain.c

!IF  "$(CFG)" == "iup - Win32 Debug"

# ADD CPP /nologo /W4 /TC /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\win\winproc.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winresiz.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winset.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winsize.c
# End Source File
# Begin Source File

SOURCE=..\src\win\wintimer.c

!IF  "$(CFG)" == "iup - Win32 Debug"

# ADD CPP /nologo /W4 /TP /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=..\src\win\wintips.c
# End Source File
# Begin Source File

SOURCE=..\src\win\wintoggle.c
# End Source File
# Begin Source File

SOURCE=..\src\win\winver.c
# End Source File
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\win\win.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winbutton.h
# End Source File
# Begin Source File

SOURCE=..\src\win\wincreat.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winframe.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winhandle.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winicon.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winproc.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winresiz.h
# End Source File
# Begin Source File

SOURCE=..\src\win\wintips.h
# End Source File
# Begin Source File

SOURCE=..\src\win\wintoggle.h
# End Source File
# Begin Source File

SOURCE=..\src\win\winver.h
# End Source File
# End Group
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\icalcsize.c
# End Source File
# Begin Source File

SOURCE=..\src\icall.c
# End Source File
# Begin Source File

SOURCE=..\src\icbox.c
# End Source File
# Begin Source File

SOURCE=..\src\icpi.c
# End Source File
# Begin Source File

SOURCE=..\src\icreate.c
# End Source File
# Begin Source File

SOURCE=..\src\idlglist.c
# End Source File
# Begin Source File

SOURCE=..\src\ienv.c
# End Source File
# Begin Source File

SOURCE=..\src\ifocus.c
# End Source File
# Begin Source File

SOURCE=..\src\ifunc.c
# End Source File
# Begin Source File

SOURCE=..\src\iglobalenv.c
# End Source File
# Begin Source File

SOURCE=..\src\ikey.c
# End Source File
# Begin Source File

SOURCE=..\src\ilex.c
# End Source File
# Begin Source File

SOURCE=..\src\imask.c
# End Source File
# Begin Source File

SOURCE=..\src\inames.c
# End Source File
# Begin Source File

SOURCE=..\src\iparse.c
# End Source File
# Begin Source File

SOURCE=..\src\ipredial.c
# End Source File
# Begin Source File

SOURCE=..\src\isbox.c
# End Source File
# Begin Source File

SOURCE=..\src\iscanf.c
# End Source File
# Begin Source File

SOURCE=..\src\ispin.c
# End Source File
# Begin Source File

SOURCE=..\src\istrutil.c
# End Source File
# Begin Source File

SOURCE=..\src\itable.c
# End Source File
# Begin Source File

SOURCE=..\src\itraverse.c
# End Source File
# Begin Source File

SOURCE=..\src\itree.c
# End Source File
# Begin Source File

SOURCE=..\src\iup.c
# End Source File
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\icall.h
# End Source File
# Begin Source File

SOURCE=..\src\icpi.h
# End Source File
# Begin Source File

SOURCE=..\src\idlglist.h
# End Source File
# Begin Source File

SOURCE=..\src\idrv.h
# End Source File
# Begin Source File

SOURCE=..\src\ifunc.h
# End Source File
# Begin Source File

SOURCE=..\src\iglobal.h
# End Source File
# Begin Source File

SOURCE=..\src\iglobalenv.h
# End Source File
# Begin Source File

SOURCE=..\src\ilex.h
# End Source File
# Begin Source File

SOURCE=..\src\imacros.h
# End Source File
# Begin Source File

SOURCE=..\src\imask.h
# End Source File
# Begin Source File

SOURCE=..\src\inames.h
# End Source File
# Begin Source File

SOURCE=..\src\ipredial.h
# End Source File
# Begin Source File

SOURCE=..\src\istrutil.h
# End Source File
# Begin Source File

SOURCE=..\src\itable.h
# End Source File
# Begin Source File

SOURCE=..\src\itree.h
# End Source File
# End Group
# End Group
# End Group
# Begin Group "inc"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=..\include\iup.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcbs.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcompat.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcpi.h
# End Source File
# Begin Source File

SOURCE=..\include\iupdef.h
# End Source File
# Begin Source File

SOURCE=..\include\iupkey.h
# End Source File
# End Group
# End Target
# End Project

