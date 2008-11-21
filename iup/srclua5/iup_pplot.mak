PROJNAME = iup
LIBNAME = iuplua_pplot51
OPT = YES
DEF_FILE = iuplua_pplot5.def

IUP := ..

DEFINES = IUPLUA_USELOH

USE_IUPLUA = Yes
USE_CDLUA = Yes
LIBS = iup_pplot

USE_LUA51 = Yes

LOHDIR = loh
SRCLUA = pplot.lua
EC = pplotfuncs.c
GC := $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := $(GC) $(EC)
