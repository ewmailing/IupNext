PROJNAME = iup
APPNAME = iupview
OPT = YES

SRC = iup_view.c

IUP := ..

LINKER = $(CPPC)

USE_CD = Yes
USE_IUPCONTROLS = Yes
USE_IUP = Yes

ifdef USE_GTK
  APPNAME = iupviewgtk
endif

ifeq "$(TEC_UNAME)" "SunOS510x86"
  DEFINES = USE_NO_OPENGL
else  
  USE_OPENGL = Yes
endif

USE_IM = Yes
ifdef USE_IM
  DEFINES += USE_IM  
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    LIBS = iupim iupimglib
  else
    IUPLIB = $(IUP)/lib/$(TEC_UNAME)
    SLIB = $(IUPLIB)/libiupim.a $(IUPLIB)/libiupimglib.a
  endif             
endif 

USE_STATIC = Yes

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += ../etc/iup.rc
endif

INCLUDES = ../src
