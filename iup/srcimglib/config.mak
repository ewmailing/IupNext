PROJNAME = iup
LIBNAME = iupimglib

#Turn off optimization to speed up compilation
#OPT = YES

INCLUDES = ../include ../src

ifdef USE_IUP_IMGLIB_LARGE
  # if you want to include images larger than 48x48 (inclusive)
  DEFINES += IUP_IMGLIB_LARGE
endif

SRC = iup_image_library.c iup_imglib_basegtk16x16.c iup_imglib_logos48x48.c iup_imglib_logos32x32.c

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += iup_imglib_basewin16x16.c iup_imglib_basewin32x32.c iup_imglib_iconswin48x48.c
else
  # Used only by the Motif driver, but must include for all Posix because it is only one lib */
  SRC += iup_imglib_basemot16x16_8bpp.c iup_imglib_logos48x48_8bpp.c iup_imglib_logos32x32_8bpp.c
endif

LIBS = iup
LDIR = ../lib/$(TEC_UNAME)

ifneq ($(findstring AIX, $(TEC_UNAME)), )
  DEFINES += IUP_IMGLIB_DUMMY
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
