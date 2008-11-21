PROJNAME = iup
LIBNAME = iupcontrols
#OPT = YES
DBG = Yes

INCLUDES = ../include ../src .

SRCCOLOR = iup_colorbrowser.c iup_colorhsi.c iup_colordlg.c
SRCCOLOR := $(addprefix color/, $(SRCCOLOR))

SRCTREE = itdraw.c itgetset.c iuptree.c itfind.c itimage.c \
          itkey.c itmouse.c itcallback.c itscroll.c itedit.c
SRCTREE := $(addprefix tree/, $(SRCTREE))

SRCMATRIX = imkey.c immark.c imaux.c immem.c immouse.c imnumlc.c \
            imcolres.c imdraw.c imfocus.c imgetset.c iupmatrix.c \
            imscroll.c imedit.c
SRCMATRIX := $(addprefix matrix/, $(SRCMATRIX))

SRC = iup_cdutil.c iup_gauge.c iup_cells.c iup_colorbar.c iup_controls.c \
      iup_dial.c iup_oldtabs.c iup_oldval.c iup_oldmask.c \
      $(SRCCOLOR) $(SRCMATRIX) $(SRCTREE)

LIBS = iup iupcd
LDIR = ../lib/$(TEC_UNAME)
USE_CD = Yes
