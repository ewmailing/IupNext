# Microsoft Developer Studio Project File - Name="iupcontrols" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iupcontrols - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupcontrols.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupcontrols.mak" CFG="iupcontrols - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupcontrols - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupcontrols - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\include" /I "..\src" /I "..\srccontrols" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "CD_NO_OLD_INTERFACE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\include" /I "..\src" /I "..\srccontrols" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "CD_NO_OLD_INTERFACE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
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

# Name "iupcontrols - Win32 Debug"
# Begin Group "iuptree"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\tree\itcallback.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itdraw.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itedit.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itfind.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itgetset.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itimage.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itkey.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itlimits.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itmouse.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itscroll.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\iuptree.c
# End Source File
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\tree\itcallback.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itdraw.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itfind.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itgetset.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itimage.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itkey.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itlimits.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itmouse.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\itscroll.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\treecd.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\tree\treedef.h
# End Source File
# End Group
# End Group
# Begin Group "iupmatrix"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\matrix\imaux.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imcolres.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imdraw.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imedit.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imfocus.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imgetset.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imkey.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immark.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immem.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immouse.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imnumlc.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imscroll.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\iupmatrix.c
# End Source File
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\matrix\imaux.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imcolres.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imdraw.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imfocus.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imgetset.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imkey.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immark.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immem.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\immouse.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imnumlc.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\imscroll.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\matridef.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\matrix\matrixcd.h
# End Source File
# End Group
# End Group
# Begin Group "iupmask"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\mask\imask_match.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\mask\imask_parse.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\mask\iupmask.c
# End Source File
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\mask\imask.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\mask\imask_match.h
# End Source File
# End Group
# End Group
# Begin Group "iupcb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\color\icb_hls.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\color\icb_rgb.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\color\iupcb.c
# End Source File
# End Group
# Begin Group "iupgc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srccontrols\color\igc_lng.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\color\iupgc.c
# End Source File
# End Group
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\iupcb.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcells.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcolorbar.h
# End Source File
# Begin Source File

SOURCE=..\include\iupcontrols.h
# End Source File
# Begin Source File

SOURCE=..\include\iupdial.h
# End Source File
# Begin Source File

SOURCE=..\include\iupgauge.h
# End Source File
# Begin Source File

SOURCE=..\include\iupgc.h
# End Source File
# Begin Source File

SOURCE=..\include\iupgetparam.h
# End Source File
# Begin Source File

SOURCE=..\include\iupmask.h
# End Source File
# Begin Source File

SOURCE=..\include\iupmatrix.h
# End Source File
# Begin Source File

SOURCE=..\include\iupsbox.h
# End Source File
# Begin Source File

SOURCE=..\include\iuptabs.h
# End Source File
# Begin Source File

SOURCE=..\include\iuptree.h
# End Source File
# Begin Source File

SOURCE=..\include\iupval.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\srccontrols\icontrols.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupcdutil.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupcdutil.h
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupcells.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupcolorbar.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupcontrols.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupdial.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupgauge.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupgetparam.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iuptabs.c
# End Source File
# Begin Source File

SOURCE=..\srccontrols\iupval.c
# End Source File
# End Target
# End Project

