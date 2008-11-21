------------------------------------------------------------------------------
-- Image class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "image",
  parent = WIDGET,
  creation = "nns", -- fake definition
  callback = {},
  createfunc = [[ 
#include<stdlib.h>
static int Image (lua_State * L)
{
  int w, h, i, j;
  char *img;
  Ihandle *image;
  
  h = luaL_getn(L, 1);
  lua_pushnumber(L, 1);
  lua_gettable(L, 1);
  w = luaL_getn(L, -1);
  lua_pop(L, 1);
  
  img = (char *) malloc (h*w);

  for (i=1; i<=h; i++)
  {
    lua_pushnumber(L, i);
    lua_gettable(L, 1);
    for (j=1; j<=w; j++)
    {
      int idx = (i-1)*w+(j-1);
      lua_pushnumber(L, j);
      lua_gettable(L, -2);
      img[idx] = (char)lua_tonumber(L, -1);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }
  
  image = IupImage(w,h,img);  
  free(img);

  w = luaL_getn(L, 2);

  for(i=1; i<=w; i++)
  {
    lua_pushnumber(L,i);
    lua_pushnumber(L,i);
    lua_gettable(L, 2);
    IupStoreAttribute(image, (char *) lua_tostring(L,-2), (char *) lua_tostring(L,-1));
    lua_pop(L, 2);
  }
  
  iuplua_plugstate(L, image);
  iuplua_pushihandle_raw(L, image);
  return 1;
} 
 
]]
}

function ctrl.createElement(class, arg)
   return Image(arg, arg.colors)
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
