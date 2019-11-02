PROJNAME = iup
APPNAME := iupvled
OPT = YES

SRC = iup_vled.c iup_vled_export.c

IUP := ..

LINKER = $(CPPC)

USE_IUPCONTROLS = Yes
USE_IUP3 = Yes

ifeq "$(TEC_SYSNAME)" "Haiku"
  USE_HAIKU = Yes
else
  USE_STATIC = Yes
ifdef GTK_DEFAULT
  ifdef USE_MOTIF
    # Build Motif version in Linux and BSD
    APPNAME := $(APPNAME)mot
  endif
else  
  ifdef USE_GTK
    # Build GTK version in IRIX,SunOS,AIX,Win32
    APPNAME := $(APPNAME)gtk
  endif
endif
endif


ifeq "$(TEC_UNAME)" "SunOS510x86"
  DEFINES += USE_NO_OPENGL
else  
  USE_OPENGL = Yes
endif

USE_IM = Yes
ifdef USE_IM
  DEFINES += USE_IM  
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    LIBS = iupim iupimglib iup_scintilla
  else
    ifdef USE_STATIC
      ifdef DBG_DIR
        IUPLIB = $(IUP)/lib/$(TEC_UNAME)d
      else
        IUPLIB = $(IUP)/lib/$(TEC_UNAME)
      endif  
      SLIB = $(IUPLIB)/libiupim.a $(IUPLIB)/libiupimglib.a $(IUPLIB)/libiup_scintilla.a
    else
      LIBS = iupim iupimglib iup_scintilla
    endif             
  endif             
endif 

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += ../etc/iup.rc
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LIBS += fontconfig
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  LIBS += fontconfig
endif

INCLUDES = ../src
