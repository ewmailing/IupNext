#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include "lualib.h"
#include <iup.h>
#include <iupcb.h>
#include <iuplua.h>
#include <luacb.h>
#include <string.h>
#include <iupmask.h>

void teste(void)
{
	printf("teste\n");
}

int main(void) {
  IupOpen();
  IupColorBrowserOpen();
  lua_open();
	//lua_baselibopen();
	//lua_userinit();
  lua_iolibopen();
  lua_strlibopen();
  lua_mathlibopen();
  //iuplua_open();
	//cblua_open();

	//lua_register("teste", teste);
	
  if(lua_dofile("/home/n/mark/iupold/tests/iupcb/a.lua") != 0)
    printf("erro lendo o arq a.lua\n");
	
  IupMainLoop();
  return 0;
}
