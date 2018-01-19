PROJNAME = iup
LIBNAME = iupcontrols
OPT = YES

INCLUDES = ../include ../src . ../srccd

SRCMATRIX = iupmat_key.c iupmat_mark.c iupmat_aux.c iupmat_mem.c iupmat_mouse.c iupmat_numlc.c \
            iupmat_colres.c iupmat_draw.c iupmat_getset.c iupmatrix.c \
            iupmat_scroll.c iupmat_edit.c iupmat_ex.c
SRCMATRIX := $(addprefix matrix/, $(SRCMATRIX))

SRC = iup_controls.c \
      iup_cells.c \
      iup_matrixlist.c \
      $(SRCMATRIX)

LIBS = iup iupcd
LDIR = ../lib/$(TEC_UNAME)
USE_CD = Yes

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
