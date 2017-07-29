PROJNAME = iup
LIBNAME  = iuplua

OPT = YES
NO_LUALINK = Yes
USE_BIN2C_LUA = Yes
NO_LUAOBJECT = Yes
LUAMOD_DIR = Yes

DEF_FILE = iuplua5.def

ifdef USE_LUA52
  LUASFX = 52
  DEFINES += LUA_COMPAT_MODULE
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
LIBNAME := $(LIBNAME)$(LUASFX)

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

CTRLUA = button.lua canvas.lua dialog.lua \
       filedlg.lua fill.lua frame.lua hbox.lua \
       item.lua image.lua label.lua menu.lua multiline.lua \
       list.lua separator.lua radio.lua \
       submenu.lua text.lua toggle.lua vbox.lua zbox.lua timer.lua \
       sbox.lua spin.lua spinbox.lua cbox.lua
       
GC = $(addsuffix .c, $(basename $(CTRLUA)))
EC = iuplua.c scanf.c iuplua_api.c
SRCLUA = iuplua.lua constants.lua $(CTRLUA)

DEFINES += IUPLUA_USELH
USE_LH_SUBDIR = Yes
LHDIR = lh

$(GC) : %.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC = $(GC) $(EC)
