PROJNAME = iup
LIBNAME = iupluacontrols51
OPT = YES
DEF_FILE = iupluacontrols5.def

IUP := ..

DEFINES = IUPLUA_USELOH

USE_IUPLUA = Yes
USE_CDLUA = Yes
LIBS = iupcontrols

USE_LUA51 = Yes

LOHDIR = loh
SRCLUA = dial.lua gauge.lua colorbrowser.lua colorbar.lua matrix.lua tree.lua cells.lua
ifndef BUILD_IUP3
  SRCLUA += val.lua tabs.lua
endif
EC = mask.c controls.c treefuncs.c matrixfuncs.c getparam.c gc.c
GC := $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := $(GC) $(EC)
