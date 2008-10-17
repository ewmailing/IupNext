# Microsoft Developer Studio Project File - Name="iuplua5exe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=iuplua5exe - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iuplua5exe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iuplua5exe.mak" CFG="iuplua5exe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iuplua5exe - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iuplua5exe - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin"
# PROP BASE Intermediate_Dir "..\obj\$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\$(ProjectName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /I "..\..\im\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /D "IUPLUA_IMGLIB" /D "USE_STATIC" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /I "..\..\im\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "_CRT_SECURE_NO_DEPRECATE" /D "IUPLUA_IMGLIB" /D "USE_STATIC" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /tlb"..\bin\iuplua5.tlb" /win32 
# ADD MTL /nologo /tlb"..\bin\iuplua5.tlb" /win32 
# ADD BASE RSC /l 1046 /d "_DEBUG" 
# ADD RSC /l 1046 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib iup.lib comctl32.lib iupcontrols.lib iupimglib.lib iup_pplot.lib iupgl.lib cd.lib iupcd.lib freetype6.lib cdgdiplus.lib lua5.1.lib iuplua51.lib iupluaim51.lib iupluaimglib51.lib iupluagl51.lib iupluacontrols51.lib iuplua_pplot51.lib iupim.lib opengl32.lib glu32.lib gdiplus.lib im.lib cdlua51.lib iupluacd51.lib imlua51.lib imlua_process51.lib cdluaim51.lib im_process.lib /nologo /out:"..\bin\iuplua51.exe" /incremental:no /libpath:"..\..\im\lib" /libpath:"..\..\cd\lib" /libpath:"..\lib" /libpath:"..\..\lua5.1\lib\vc9" /debug /pdbtype:sept /subsystem:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib iup.lib comctl32.lib iupcontrols.lib iupimglib.lib iup_pplot.lib iupgl.lib cd.lib iupcd.lib freetype6.lib cdgdiplus.lib lua5.1.lib iuplua51.lib iupluaim51.lib iupluaimglib51.lib iupluagl51.lib iupluacontrols51.lib iuplua_pplot51.lib iupim.lib opengl32.lib glu32.lib gdiplus.lib im.lib cdlua51.lib iupluacd51.lib imlua51.lib imlua_process51.lib cdluaim51.lib im_process.lib /nologo /out:"..\bin\iuplua51.exe" /incremental:no /libpath:"..\..\im\lib" /libpath:"..\..\cd\lib" /libpath:"..\lib" /libpath:"..\..\lua5.1\lib\vc9" /debug /pdbtype:sept /subsystem:console /MACHINE:I386

!ENDIF

# Begin Target

# Name "iuplua5exe - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\srcconsole\console5.lua
# End Source File
# Begin Source File

SOURCE=..\srcconsole\iuplua5.rc
# End Source File
# Begin Source File

SOURCE=..\srcconsole\iuplua51.c
# End Source File
# End Group
# End Target
# End Project

