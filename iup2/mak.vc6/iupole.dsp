# Microsoft Developer Studio Project File - Name="iupole" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iupole - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupole.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupole.mak" CFG="iupole - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupole - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupole - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
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

# Name "iupole - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=..\include\iupole.h
# End Source File
# Begin Source File

SOURCE=..\srcole\legacy.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tambientproperties.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tDispatch.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tIupCtl.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleClientSite.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleControl.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleControlSite.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleHandler.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleInPlaceFrame.h
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleInPlaceSite.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\srcole\iupolectl.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\legacy.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tambientproperties.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tDispatch.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tiupctl.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleClientSite.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleControl.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tolecontrolsite.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleInPlaceFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\srcole\tOleInPlaceSite.cpp
# End Source File
# End Group
# End Target
# End Project

