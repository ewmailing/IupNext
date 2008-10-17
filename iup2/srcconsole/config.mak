PROJNAME = iup
APPNAME = iuplua51
APPTYPE = console

LOHDIR = loh
SRCLUA = console5.lua
SRC = iuplua51.c


# Disable strip
STRIP = 
# Optimize
OPT = YES      
# DBG = Yes


ifdef DBG
  # Statically link everything only when debugging
  IUP := ..
  USE_IUPLUA = Yes
  USE_IUP = Yes
  USE_STATIC = Yes
  USE_LUA51 = Yes

  # IM and IupPPlot uses C++
  LINKER = $(CPPC)
  
  ifeq "$(TEC_UNAME)" "SunOS510x86"
    IUPLUA_NO_GL = Yes
  endif
    
  #IUPLUA_NO_GL = Yes
  ifndef IUPLUA_NO_GL 
    USE_OPENGL = Yes
  else
    DEFINES = IUPLUA_NO_GL
  endif

  #IUPLUA_NO_IM = Yes
  ifndef IUPLUA_NO_IM
    USE_IMLUA = Yes
    LIBS += iupim iupluaim$(LIBLUASUFX) imlua_process$(LIBLUASUFX) im_process
  else
    DEFINES += IUPLUA_NO_IM
  endif

  #IUPLUA_NO_CD = Yes
  ifndef IUPLUA_NO_CD 
    USE_CDLUA = Yes
    USE_IUPCONTROLS = Yes
    LIBS += iup_pplot iuplua_pplot$(LIBLUASUFX)
    ifndef IUPLUA_NO_IM
      LIBS += cdluaim$(LIBLUASUFX)
    endif
    ifneq ($(findstring Win, $(TEC_SYSNAME)), )
      USE_GDIPLUS=Yes
    else
  #    USE_XRENDER=Yes
    endif
  else
    DEFINES += IUPLUA_NO_CD
  endif

  IUPLUA_IMGLIB = Yes
  ifdef IUPLUA_IMGLIB
    DEFINES += IUPLUA_IMGLIB
    LIBS += iupimglib
  endif
else
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    # Dinamically link in Windows, when not debugging
    # Must call "tecmake dll8"
    USE_LUA51 = Yes
    USE_DLL = Yes
    GEN_MANIFEST = No
  else
    # In UNIX Lua is always statically linked, late binding is used.
    USE_STATIC = Yes
    USE_LUA51 = Yes
  endif
endif


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SLIB += setargv.obj
  SRC += iuplua5.rc
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LDFLAGS = -s
  LIBS += readline history
endif

ifneq ($(findstring Linux, $(TEC_UNAME)), )
  LIBS += dl 
  #To allow late binding
  LFLAGS = -Wl,-E
  LIBS += readline history curses ncurses
endif

ifneq ($(findstring BSD, $(TEC_UNAME)), )
  #To allow late binding
  LFLAGS = -Wl,-E
  LIBS += readline history curses ncurses
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  LIBS += dl
endif

#ifneq ($(findstring AIX, $(TEC_UNAME)), )
#  FLAGS  += -mminimal-toc
#  OPTFLAGS = -mminimal-toc -ansi -pedantic 
#  LFLAGS = -Xlinker "-bbigtoc"
#endif
