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
SRCLUA = val.lua dial.lua gauge.lua colorbrowser.lua colorbar.lua tabs.lua sbox.lua matrix.lua tree.lua spin.lua spinbox.lua cbox.lua cells.lua
EC = mask.c controls.c treefuncs.c matrixfuncs.c getparam.c gc.c
GC := $(addsuffix .c, $(basename $(SRCLUA)))

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := $(GC) $(EC)
