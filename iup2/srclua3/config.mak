#BUILD_IUP3 = Yes

PROJNAME = iup
LIBNAME = iuplua3
OPT = YES

LOHDIR = loh
SRCLUA = iuplua.lua iuplua_widgets.lua constants.lua luaspin.lua luasbox.lua
ifdef BUILD_IUP3
  SRCLUA += luaval.lua luatabs.lua
endif
SRC    = iuplua.c iuplua_api.c iuplua_widgets.c scanf.c luacbox.c luasbox.c luaspin.c
ifdef BUILD_IUP3
  SRC += luaval.c luatabs.c luagc.c luagetparam.c
endif

USE_LUA = Yes

DEFINES = IUPLUA_USELOH
INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  USE_MOTIF = Yes
endif

