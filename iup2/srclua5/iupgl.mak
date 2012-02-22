PROJNAME = iup
LIBNAME = iupluagl

OPT = YES
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes
NO_LUAOBJECT = Yes

DEF_FILE = iupluagl5.def

IUP := ..

# Can not use USE_IUPLUA because Tecmake will include "iupluagl5X" in linker

USE_IUP = Yes
USE_OPENGL = Yes

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

DEFINES += IUPLUA_USELH
USE_LH_SUBDIR = Yes
LHDIR = lh

LIBS = iuplua$(LUASFX)

SRCLUA = glcanvas.lua
GC = $(addsuffix .c, $(basename $(SRCLUA)))

EC = glcanvasfuncs.c 

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= $(GC) $(EC)
