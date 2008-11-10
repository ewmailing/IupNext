APPNAME = iuptest
APPTYPE = CONSOLE

INCLUDES = ../include

ifdef USE_GTK
  APPNAME = iuptestgtk
endif

USE_IUP = Yes
USE_STATIC = Yes
IUP = ..

DBG = Yes

#SRC = tray.c
#SRC = dialog.c
SRC = predialogs.c
#SRC = timer.c
#SRC = label.c
#SRC = canvas.c
#SRC = frame.c
#SRC = idle.c
#SRC = button.c
#SRC = gauge.c
#SRC = toggle.c
#SRC = vbox.c
#SRC = hbox.c
#SRC = progressbar.c
#SRC = text.c
#SRC = val.c
#SRC = tabs.c
#SRC = sample.c
#SRC = menu.c
#SRC = spin.c
#SRC = text_spin.c
#SRC = list.c

#ifneq ($(findstring Win, $(TEC_SYSNAME)), )
#  LIBS += iupimglib
#else
#  SLIB += $(IUP)/lib/$(TEC_UNAME)/libiupimglib.a
#endif

#USE_CD = Yes
#SRC = canvas_scrollbar.c
#SRC = canvas_dbuffer.c

#USE_OPENGL = Yes
#SRC = glcanvas.c
#SRC = glcanvas_cube.c

USE_IUPCONTROLS = Yes
#SRC = tabs.c
#SRC = val.c
#SRC = colorbrowser.c
#SRC = dial.c
#SRC = colorbar.c
#SRC = cells_numbering.c
#SRC = cells_degrade.c
#SRC = cells_checkboard.c
#SRC = gauge.c

#USE_IUPCONTROLS = Yes
#LINKER = g++
#SRC = pplot.c
#ifneq ($(findstring Win, $(TEC_SYSNAME)), )
#  LIBS += iup_pplot cdpdflib
#else
#  SLIB += $(IUP)/lib/$(TEC_UNAME)/libiup_pplot.a $(CD)/lib/$(TEC_UNAME)/libcdpdflib.a
#endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += iuptest.rc 
else
  ifneq ($(findstring cygw, $(TEC_UNAME)), )
    SRC += iuptest.rc
  endif
endif
 
#ifneq ($(findstring Win, $(TEC_SYSNAME)), )
#  USE_GDIPLUS=Yes
#else
#  USE_XRENDER=Yes
#endif
