PROJNAME = iup
APPNAME = iupluascripter

STRIP = 
OPT = YES      
NO_SCRIPTS = Yes
# IM and IupPlot uses C++
LINKER = $(CPPC)
# To use a subfolder with the Lua version for binaries
LUAMOD_DIR = Yes

NO_LUAOBJECT = Yes
USE_BIN2C_LUA = Yes

ifdef USE_LUA_VERSION
  USE_LUA51:=
  USE_LUA52:=
  USE_LUA53:=
  ifeq ($(USE_LUA_VERSION), 53)
    USE_LUA53:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 52)
    USE_LUA52:=Yes
  endif
  ifeq ($(USE_LUA_VERSION), 51)
    USE_LUA51:=Yes
  endif
endif

ifdef USE_LUA53
  LUASFX = 53
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    ifneq ($(findstring x86, $(TEC_UNAME)), )
      FLAGS = -std=gnu99
    endif
  endif
else
ifdef USE_LUA52
  LUASFX = 52
else
  USE_LUA51 = Yes
  LUASFX = 51
endif
endif

APPNAME := $(APPNAME)$(LUASFX)

ifdef NO_LUAOBJECT
  DEFINES += IUPLUA_USELH
  USE_LH_SUBDIR = Yes
  LHDIR = lh
else
  DEFINES += IUPLUA_USELOH
  USE_LOH_SUBDIR = Yes
  LOHDIR = loh$(LUASFX)
endif

INCLUDES = ../include

USE_IUP = Yes
USE_IUPLUA = Yes

IUP = ..

SRC = iupluascripter.c

ifdef DBG_DIR
  SFX=d
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS += iupimglib iup_scintilla imm32 iupluascripterdlg$(LUASFX)
else
  SLIB += $(IUP)/lib/$(TEC_UNAME)$(SFX)/libiup_scintilla.a \
          $(IUP)/lib/$(TEC_UNAME)$(SFX)/libiupimglib.a \
          $(IUP)/lib/$(TEC_UNAME)$(SFX)/Lua$(LUASFX)\libiupluascripterdlg$(LUASFX).a
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  INCLUDES += $(IUP)/etc
  SRC += iupluascripter.rc
endif
