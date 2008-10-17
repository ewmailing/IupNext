PROJNAME = iup
LIBNAME = iupspeech
OPT = YES

INCLUDES =  ../include ../src C:/LNG/VC7/ATLMFC/INCLUDE C:/LNG/VC7/PlatformSDK/INCLUDE C:/LNG/SpeechSDK/include
LDIR = ../lib/$(TEC_UNAME)  

SRC = iupspeech.cpp
INC =	iupspeech.h

COMPILER = $(VC7)
ifeq "$(TEC_UNAME)" "dll"
  DEFINES += _IUPDLL
endif

LDIR += C:/LNG/VC7/ATLMFC/LIB C:/LNG/SpeechSDK/LIB/i386
LIBS = iup
