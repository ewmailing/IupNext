PROJNAME = iup
LIBNAME = iupluaimglib51
OPT = YES

SRC = iupluaimglib.c
DEF_FILE = iupluaimglib5.def

INCLUDES = ../include
LIBS = iupimglib
LDIR = ../lib/$(TEC_UNAME)

IUP := ..

USE_IUPLUA = Yes
USE_LUA51 = Yes
