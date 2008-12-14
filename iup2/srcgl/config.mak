PROJNAME = iup
LIBNAME  = iupgl
OPT = YES

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

USE_OPENGL = Yes

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC = iupglw.c
else
  SRC = iupglx.c
  USE_MOTIF = Yes
  
  ifneq ($(findstring Linux, $(TEC_UNAME)), )
    INCLUDES += .
    SRC += GL/GLwMDrawA.c
  endif
  ifneq ($(findstring cygw, $(TEC_UNAME)), )
    INCLUDES += .
    SRC += GL/GLwMDrawA.c
  endif
  ifneq ($(findstring SunOS510, $(TEC_UNAME)), )
    INCLUDES += .
    SRC += GL/GLwMDrawA.c
  endif
  
endif
