#BUILD_IUP3 = Yes

PROJNAME = iup
LIBNAME  = iupluacontrols3
OPT = YES

USE_LUA  = Yes
USE_CDLUA = Yes

LOHDIR = loh
SRCLUA =  luadial.lua luagauge.lua luacolorbar.lua \
          luamatrix.lua luatree.lua luacb.lua luacells.lua
ifndef BUILD_IUP3
  SRCLUA += luaval.lua luatabs.lua
endif
SRC    =  luadial.c luagauge.c luacb.c luacolorbar.c luacells.c \
          luamask.c luacontrols.c luamatrix.c luatree.c
ifndef BUILD_IUP3
  SRC += luaval.c luatabs.c luagc.c luagetparam.c
endif

DEFINES = IUPLUA_USELOH
INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupcontrols iupcd

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  USE_MOTIF = Yes
endif
