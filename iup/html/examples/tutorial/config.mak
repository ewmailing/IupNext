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

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS += iupimglib im_process iupim
else
  SLIB += $(IUP)/lib/$(TEC_UNAME)/libiupim.a $(IUP)/lib/$(TEC_UNAME)/libiupimglib.a $(IM)/lib/$(TEC_UNAME)/libim_process.a
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  INCLUDES += ../../../etc/
  SRC += ../../../etc/iup.rc
endif
