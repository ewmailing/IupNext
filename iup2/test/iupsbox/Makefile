#
# 1) A ordem de inclusao das libs, includes e' invertido, ou seja:
# LIBS = iup cd
# faz com que o cd seja incluido antes do iup.
#
# 2) Variaveis de ambiente sao utilizadas a todo momento para
# indicar a localizacao da bibliotecas e includes. Isso e' interessante 
# pois cada maquina pode ter a biblioteca instalada em um local. 
# Ex: $(IUPINC) 
# [kobke:~] env | grep IUPINC
# IUPINC=/home/t/tecgraf/lib/iup/include
#
# TEC_UNAME indica dentro do Tecgraf o sistema (Linux, Solaris, etc...)
#

MEUTESTE = test2

# Bibliotecas Tecgraf
MYIUP = /home/t/tecgraf/lib/iup
#MYIUP = /home/n/mark/iup
MYIUPLIB = $(MYIUP)/lib/${TEC_UNAME}
MYIUPINC = $(MYIUP)/include
IUP    = $(MYIUPLIB)/libiup.a
IUPCTL = $(MYIUPLIB)/libiupcontrols.a
IUPLUA = $(MYIUPLIB)/libiuplua.a
IUPLUA4 = $(MYIUPLIB)/libiuplua4.a
IUPGL  = $(MYIUPLIB)/libiupgl.a

# Outras bibliotecas tecgraf
LUA    = $(LUALIB)/liblua.a $(LUALIB)/liblualib.a
CD     = $(CDLIB)/libcdiup.a $(CDLIB)/libcd.a

# Bibliotecas OpenGL
#GLDIR  = /usr/X11R6
GLDIR  = /home/t/tecgraf/libpath/$(TEC_UNAME)
#GL     = $(IUPGL) -lGLw -lGL -lGLU -lGLwM
GL     = $(IUPGL) -lGL -lGLU #-lGLwM
GLINC = /usr/X11R6/include

# Bibliotecas do X e Motif
XDIR = /usr/X11R6/lib
X    = -lXm -lXt -lXmu -lX11 -lm
#X    = -lXt -lXmu -lX11 -lXm -lXpm

# Esses sao os diretorios usados para procurar os arquivos .h
INCLUDES = -I$(MYIUPINC) -I$(IUPINC) -I$(MYIUPINC) -I$(IUPLUAINC) -I$(LUAINC) -I$(CDINC) -I$(GLINC)

# Compilador a ser usado
CC = gcc

# Opcoes de compilacao. No caso:
# -Wall e -w significa mostrar todos os warnings possiveis (boa pratica)
# -g para compilar em modo debug
CFLAGS = -g -W -Wall $(INCLUDES)

# Une as bibliotecas anteriormente especificadas em LIBS
#LIBS   = $(IUP) $(X)
#LIBS   = $(IUPLUA) $(IUP) $(LUA) $(X)
LIBS   = $(IUPGL) $(GL) $(IUPCTL) $(CD) $(IUP) $(X)
#LIBS   = $(IUPLUA) $(IUPCTL) $(CD) $(IUP) $(LUA) $(X)

# parametros passados para o make

# A opcao make all executa o comando espeficiado para o MEUTESTE
all: $(MEUTESTE)

# Meu teste compila o .c definido na variavel MEUTESTE em um executavel
# utilizando o nome definido em MEUTESTE
$(MEUTESTE): $(MEUTESTE).c
	$(CC) $(CFLAGS) -o $@ $< -L$(XDIR) $(LIBS) -L$(GLDIR)

# A opcao make clena remove tudo que foi gerado a partir do make: o
# core (se existir), o .o e o executavel
clean:
	rm -f core $(MEUTESTE)

# A opcao make re faz um rebuild, que consiste em um make clean e um
# make all
re: clean all ;

