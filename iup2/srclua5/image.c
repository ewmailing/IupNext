/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


 
#include<stdlib.h>
static int Image (lua_State * L)
{
  int w, h, i, j;
  unsigned char *img;
  Ihandle *image;
  
  h = luaL_getn(L, 1);
  lua_pushnumber(L, 1);
  lua_gettable(L, 1);
  w = luaL_getn(L, -1);
  lua_pop(L, 1);
  
  img = (unsigned char *) malloc (h*w);

  for (i=1; i<=h; i++)
  {
    lua_pushnumber(L, i);
    lua_gettable(L, 1);
    for (j=1; j<=w; j++)
    {
      int idx = (i-1)*w+(j-1);
      lua_pushnumber(L, j);
      lua_gettable(L, -2);
      img[idx] = (unsigned char)lua_tonumber(L, -1);
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
    IupStoreAttribute(image, lua_tostring(L,-2), lua_tostring(L,-1));
    lua_pop(L, 2);
  }
  
  iuplua_plugstate(L, image);
  iuplua_pushihandle_raw(L, image);
  return 1;
} 
 
int iupimagelua_open(lua_State * L)
{
  iuplua_register(L, Image, "Image");


#ifdef IUPLUA_USELOH
#ifdef TEC_BIGENDIAN
#ifdef TEC_64
#include "loh/image_be64.loh"
#else
#include "loh/image_be32.loh"
#endif
#else
#ifdef TEC_64
#ifdef WIN64
#include "loh/image_le64w.loh"
#else
#include "loh/image_le64.loh"
#endif
#else
#include "loh/image.loh"
#endif
#endif
#else
  iuplua_dofile(L, "image.lua");
#endif

  return 0;
}

