PROJNAME = iup
LIBNAME = iupcontrols
OPT = YES

INCLUDES = ../include ../src .

SRCCOLOR = iup_colorbrowser.c iup_colorhsi.c iup_colordlg.c
SRCCOLOR := $(addprefix color/, $(SRCCOLOR))

SRCTREE = iuptree_draw.c iuptree_getset.c iuptree.c iuptree_find.c iuptree_image.c \
          iuptree_key.c iuptree_mouse.c iuptree_callback.c iuptree_scroll.c iuptree_edit.c
SRCTREE := $(addprefix tree/, $(SRCTREE))

SRCMATRIX = iupmat_key.c iupmat_mark.c iupmat_aux.c iupmat_mem.c iupmat_mouse.c iupmat_numlc.c \
            iupmat_colres.c iupmat_draw.c iupmat_focus.c iupmat_getset.c iupmatrix.c \
            iupmat_scroll.c iupmat_edit.c
SRCMATRIX := $(addprefix matrix/, $(SRCMATRIX))

SRC = iup_cdutil.c iup_gauge.c iup_cells.c iup_colorbar.c iup_controls.c \
      iup_dial.c iup_oldtabs.c iup_oldval.c iup_oldmask.c \
      $(SRCCOLOR) $(SRCMATRIX) $(SRCTREE)

LIBS = iup iupcd
LDIR = ../lib/$(TEC_UNAME)
USE_CD = Yes

# force the definition of math functions using float
# Some compilers do not define them
ifneq ($(findstring ow, $(TEC_UNAME)), )
  DEFINES += IUP_DEFMATHFLOAT
endif         
ifneq ($(findstring bc, $(TEC_UNAME)), )
  DEFINES += IUP_DEFMATHFLOAT
endif
ifneq ($(findstring AIX, $(TEC_UNAME)), )
  DEFINES += IUP_DEFMATHFLOAT
endif
ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  DEFINES += IUP_DEFMATHFLOAT
endif
