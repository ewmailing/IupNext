PROJNAME = iup
LIBNAME = iupole
OPT = YES

INCLUDES =  ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

SRC = iupolectl.cpp \
			legacy.cpp \
			tambientproperties.cpp \
			tDispatch.cpp \
			tiupctl.cpp \
			tOleClientSite.cpp \
			tOleControl.cpp \
			tolecontrolsite.cpp \
			tOleHandler.cpp \
			tOleInPlaceFrame.cpp \
			tOleInPlaceSite.cpp


