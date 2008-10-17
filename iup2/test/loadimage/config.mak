APPNAME = loadimage
PROJNAME = loadimage

DBG = Yes

USE_IM=Yes
USE_IUP=Yes

LIBS = iupim

ifneq "$(TEC_SYSNAME)" "Win32"
  USE_STATIC = Yes
  IUPLIB = $(IUP)/lib/$(TEC_UNAME)
  
  ifdef USE_STATIC
    SLIB := $(addprefix $(IUPLIB)/lib, $(LIBS))
    SLIB := $(addsuffix .a, $(SLIB))
    LIBS = 
  endif
endif

SRC = iuploadimage.cpp
