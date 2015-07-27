PROJNAME = iup
APPNAME = simple_paint
APPTYPE = console

INCLUDES = ../include

USE_IUP3 = Yes
USE_STATIC = Yes
IUP = ../../..

LINKER = g++

SRC = simple_paint.c

#DBG = Yes

USE_CD = Yes
USE_IM = Yes

#LIBS = cdcontextplus gdiplus
#USE_OPENGL = Yes

ifdef DBG_DIR
  SFX=d
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS += iupimglib im_process iupim
else
  SLIB += $(IUP)/lib/$(TEC_UNAME)$(SFX)/libiupim.a $(IUP)/lib/$(TEC_UNAME)$(SFX)/libiupimglib.a $(IM)/lib/$(TEC_UNAME)$(SFX)/libim_process.a
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  INCLUDES += ../../../etc/
  SRC += ../../../etc/iup.rc
endif
