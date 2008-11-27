PROJNAME = iup
APPNAME = ledc
APPTYPE = console
OPT = YES

#y.tab.c y.tab.h : ledc.y
#	yacc -dv ledc.y

#lex.yy.c : ledc.l
#	lex ledc.l

INCLUDES = .

SRC = lex.yy.c y.tab.c ledc.c
