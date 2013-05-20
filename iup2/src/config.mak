PROJNAME = iup
LIBNAME = iup
OPT = YES

INCLUDES = ../include .

SRC = idlglist.c ienv.c inames.c ikey.c ifunc.c ilex.c iparse.c \
      icall.c icalcsize.c itree.c iscanf.c icpi.c ipredial.c iup.c \
      ifocus.c imask.c itable.c icreate.c itraverse.c iglobalenv.c \
      icbox.c isbox.c ispin.c istrutil.c

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC +=  win/win.c win/winfiledlg.c win/winbutton.c win/wintoggle.c \
          win/winframe.c win/wincreat.c win/windlg.c win/windraw.c win/winget.c \
          win/winicon.c win/winmain.c win/winkey.c win/winedit.c \
          win/winproc.c win/winresiz.c win/winset.c win/winsize.c win/winhandle.c \
          win/winver.c win/winhelp.c win/wintips.c win/windestroy.c win/wintimer.c \
          win/winglobal.c win/winhook.c win/wincombo.c

  INCLUDES += win

  DEFINES += _WIN32_WINNT=0x0501 _WIN32_IE=0x600 WINVER=0x0501

	ifneq ($(findstring dll, $(TEC_UNAME)), )
	  DEFINES += _IUPDLL
	  SRC += iup_dll.rc
	endif
	
	ifeq "$(TEC_UNAME)" "owc1"
	  # Necessary or IUP will not work in Open Watcom (needs to be retested)
	  FLAGS = -d2
	endif
else
  SRC += mot/motcreat.c mot/motget.c mot/motsize.c \
         mot/motif.c mot/motkey.c mot/motproc.c mot/motresiz.c \
         mot/motset.c mot/mottips.c mot/motfiledlg.c mot/mothelp.c mot/Canvas.c \
         mot/motcolor.c mot/motglobal.c mot/mottimer.c mot/motmsgdlg.c mot/utilsx11.c

  INCLUDES += mot
  
  USE_MOTIF=Yes   
  
  # Define to use Motif 1.x
  ifdef USE_MOTIF1
    SRC += mot/ComboBox1.c 
  endif

	ifeq "$(TEC_SYSNAME)" "Linux"
	  DEFINES += LINUX
	else
	  DEFINES += IUP_MOTIF_NO_REDISPLAY
	endif
	
	ifeq "$(TEC_SYSNAME)" "SunOS"
	  # Necessary or the fileopen will not work in SunOS
	  DEFINES += NO_PATH_MODE_RELATIVE
	endif
endif

