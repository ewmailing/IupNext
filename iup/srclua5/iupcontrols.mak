PROJNAME = iup
LIBNAME = iupluacontrols
OPT = YES
DEF_FILE = iupluacontrols.def

IUP := ..

DEFINES = IUPLUA_USELOH

USE_IUP3 = Yes
USE_IUPLUA = Yes
USE_CDLUA = Yes
LIBS = iupcontrols

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
SRCLUA = dial.lua gauge.lua colorbrowser.lua colorbar.lua matrix.lua cells.lua
GC := $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix il_, $(GC))

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC := iuplua_controls.c il_mask.c il_matrix_aux.c $(GC)
