PROJNAME = iup
LIBNAME = iuplua_mglplot
OPT = YES
DEF_FILE = iuplua_mglplot.def

IUP := ..

ifndef IUPLUA_NO_LOH
  DEFINES = IUPLUA_USELOH
endif

USE_IUP3 = Yes
USE_IUPLUA = Yes
LIBS = iup_mglplot

ifdef USE_LUA52
  LOHDIR = loh52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LOHDIR = loh51
  LIBNAME := $(LIBNAME)51
endif

NO_LUALINK = Yes
USE_LOH_SUBDIR = Yes
SRCLUA = mglplot.lua
USE_BIN2C_LUA=Yes

GC := $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix il_, $(GC))

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := iuplua_mglplot.c $(GC)

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_IUPLUA:=
endif
