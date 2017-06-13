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
SRC = iupluascripter.c

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

IUP = ..
USE_IUP = Yes
USE_IUPLUA = Yes

ifdef DBG
  ALL_STATIC=Yes
endif

ifdef ALL_STATIC
  # Statically link everything only when debugging
  USE_STATIC = Yes
  
  ifdef DBG_DIR
    IUP_LIB = $(IUP)/lib/$(TEC_UNAME)d
#    LUA_LIB = $(LUA)/lib/$(TEC_UNAME)d
  else
    IUP_LIB = $(IUP)/lib/$(TEC_UNAME)
#    LUA_LIB = $(LUA)/lib/$(TEC_UNAME)
  endif  
  
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla imm32
  else
    SLIB += $(IUP_LIB)/Lua$(LUASFX)/libiupluascripterdlg$(LUASFX).a \
            $(IUP_LIB)/libiup_scintilla.a \
            $(IUP_LIB)/libiupimglib.a
  endif
else
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    # Dinamically link in Windows, when not debugging
    # Must call "tecmake dll10" so USE_* will use the correct TEC_UNAME
    USE_DLL = Yes
    GEN_MANIFEST = No
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla imm32
  else
    LDIR += $(IUP_LIB)/Lua$(LUASFX)
    LIBS += iupluascripterdlg$(LUASFX) iupimglib iup_scintilla
    ifneq ($(findstring cygw, $(TEC_UNAME)), )
      # Except in Cygwin
    else
      # In UNIX Lua is always statically linked, late binding is used.
      NO_LUALINK = Yes
      SLIB += $(LUA_LIB)/liblua$(LUA_SFX).a
    endif
  endif
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  #Comment the following line to build under MingW
  ifneq ($(findstring vc, $(TEC_UNAME)), )
    SLIB += setargv.obj
  endif
  SRC += iupluascripter.rc
  INCLUDES = ../etc
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
endif

ifneq ($(findstring Linux, $(TEC_UNAME)), )
  LIBS += dl 
  #To allow late binding
  LFLAGS = -Wl,-E
  ifneq ($(findstring Linux26g4_64co, $(TEC_UNAME)), )
    LIBS += gthread-2.0
  endif
endif

ifneq ($(findstring BSD, $(TEC_UNAME)), )
  #To allow late binding
  LFLAGS = -Wl,-E
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  LIBS += dl
endif
