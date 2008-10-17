PROJNAME = iup
LIBNAME  = iupluacontrols3
OPT = YES

USE_LUA  = Yes
USE_CDLUA = Yes

LOHDIR = loh
SRCLUA =  luaval.lua luadial.lua luagauge.lua luacb.lua luacolorbar.lua luatabs.lua \
          luamatrix.lua luatree.lua luasbox.lua luaspin.lua luacells.lua
SRC    =  luaval.c luadial.c luagauge.c luagc.c luacb.c luacolorbar.c luacbox.c luacells.c \
          luatabs.c luamask.c luacontrols.c luamatrix.c luatree.c luasbox.c luaspin.c luagetparam.c

DEFINES = IUPLUA_USELOH
INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupcontrols iupcd

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  USE_MOTIF = Yes
endif
