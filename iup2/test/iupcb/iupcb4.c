#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include "lualib.h"
#include <iup.h>
#include <iupcb.h>
#include <iuplua4.h>
#include <luacb.h>
#include <string.h>
#include <iupmask.h>

void teste(void)
{
	printf("teste\n");
}

lua_State *L;

int main(void) {
  IupOpen();
  IupColorBrowserOpen();
  L = lua_open(0);
	lua_baselibopen(L);
  lua_iolibopen(L);
  lua_strlibopen(L);
  lua_mathlibopen(L);
  iuplua_open(L);
	//cblua_open();

	//lua_register("teste", teste);
	
  if(lua_dofile(L, "/home/n/mark/iupold/tests/iupcb/a.lua") != 0)
    printf("erro lendo o arq a.lua\n");
	
  IupMainLoop();
  return 0;
}
