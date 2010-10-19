PROJNAME = iup
LIBNAME = iupweb
OPT = YES

INCLUDES =  ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC = iupwin_webbrowser.cpp
  LIBS += iupole
else
  ifdef GTK_DEFAULT
    SRC = iupgtk_webbrowser.cpp
    LIBS += webkit-1.0
    INCLUDES += $(GTK)/include/webkit-1.0 $(GTK)/include/libsoup-2.4
  else
    SRC = iupmot_webbrowser.cpp
    LIBS += XmHTML
  endif
endif
