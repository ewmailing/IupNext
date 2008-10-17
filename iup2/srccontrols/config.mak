PROJNAME = iup
LIBNAME = iupcontrols
OPT = YES

INCLUDES = ../include ../src .

SRCMASK = imask_match.c imask_parse.c iupmask.c
SRCMASK := $(addprefix mask/, $(SRCMASK))
                         
SRCCOLOR = igc_lng.c iupcb.c icb_rgb.c icb_hls.c iupgc.c 
SRCCOLOR := $(addprefix color/, $(SRCCOLOR))

SRCTREE = itdraw.c itgetset.c iuptree.c itfind.c itlimits.c \
          itkey.c itmouse.c itcallback.c itscroll.c itimage.c itedit.c
SRCTREE := $(addprefix tree/, $(SRCTREE))

SRCMATRIX = imkey.c immark.c imaux.c immem.c immouse.c imnumlc.c \
            imcolres.c imdraw.c imfocus.c imgetset.c iupmatrix.c \
            imscroll.c imedit.c
SRCMATRIX := $(addprefix matrix/, $(SRCMATRIX))

SRC = iupgetparam.c iupdial.c iupgauge.c iupcontrols.c iupcells.c \
      iuptabs.c iupval.c iupcolorbar.c iupcdutil.c \
      $(SRCMASK) $(SRCTREE) $(SRCCOLOR) $(SRCMATRIX)

LIBS = iup iupcd
LDIR = ../lib/$(TEC_UNAME)

USE_CD = Yes

ifeq ($(findstring Win, $(TEC_SYSNAME)), )
  DEFINES = _MOTIF_  
  USE_MOTIF = Yes
endif
