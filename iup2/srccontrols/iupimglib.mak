PROJNAME = iup
LIBNAME = iupimglib
OPT = YES

INCLUDES = ../include ../src

SRC = iupimglib.c

LIBS = iup
LDIR = ../lib/$(TEC_UNAME)

#iupimglib.c: iupimglib.led
#	ledc -f IupImageLibOpen -o iupimglib.c iupimglib.led
