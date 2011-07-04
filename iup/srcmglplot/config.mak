PROJNAME = iup
LIBNAME  = iup_mglplot
OPT = YES

USE_OPENGL = Yes

ifdef DBG
  DEFINES += IUP_ASSERT
endif  

INCLUDES = ../include ../src .
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iupgl

DEFINES = NO_PNG NO_GSL

SRCMGLPLOT = mgl_1d.cpp mgl_crust.cpp mgl_evalc.cpp mgl_idtf.cpp \
  mgl_2d.cpp mgl_data.cpp mgl_evalp.cpp mgl_main.cpp \
  mgl_3d.cpp mgl_data_cf.cpp mgl_exec.cpp mgl_parse.cpp \
  mgl_ab.cpp mgl_data_io.cpp mgl_export.cpp mgl_pde.cpp \
  mgl_addon.cpp mgl_data_png.cpp mgl_fit.cpp mgl_tex_table.cpp \
  mgl_axis.cpp mgl_def_font.cpp mgl_flow.cpp mgl_vect.cpp \
  mgl_combi.cpp mgl_eps.cpp mgl_font.cpp mgl_zb.cpp \
  mgl_cont.cpp mgl_eval.cpp mgl_gl.cpp mgl_zb2.cpp
SRCMGLPLOT := $(addprefix mgl/, $(SRCMGLPLOT))

SRC = iup_mglplot.cpp $(SRCMGLPLOT)

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
