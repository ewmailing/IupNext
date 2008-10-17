/** \file
 * \brief Bindig of iupgl to Lua 3.
 *
 * See Copyright Notice in iup.h
 * $Id: luaglcanvas.c,v 1.1 2008-10-17 06:21:03 scuri Exp $
 */
 
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iupgl.h"

#include "iuplua.h"
#include "iupluagl.h"
#include "il.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>


static void createglcanvas(void)
{
  int tag = (int)lua_getnumber(lua_getglobal("iuplua_tag"));
  lua_pushusertag(IupGLCanvas(NULL), tag);
}

static void iscurrent(void)
{
  lua_pushnumber(IupGLIsCurrent(iuplua_checkihandle(1))); 
}

static void makecurrent(void)
{
  IupGLMakeCurrent(iuplua_checkihandle(1)); 

  iuplua_regstring_mask((char*)glGetString(GL_VENDOR), "IUPGL_VENDOR");
  iuplua_regstring_mask((char*)glGetString(GL_RENDERER), "IUPGL_RENDERER");
  iuplua_regstring_mask((char*)glGetString(GL_VERSION), "IUPGL_VERSION");
}

static void swapbuffers(void)
{
  IupGLSwapBuffers(iuplua_checkihandle(1)); 
}

static void palette(void)
{
  IupGLPalette(iuplua_checkihandle(1),
               luaL_check_int(2),
               (float)luaL_check_number(3),
               (float)luaL_check_number(4),
               (float)luaL_check_number(5));
}


int iupgllua_open (void)
{
  iuplua_register("iupCreateGLCanvas",createglcanvas);
  iuplua_register("IupGLMakeCurrent",makecurrent);
  iuplua_register("IupGLIsCurrent",iscurrent);
  iuplua_register("IupGLSwapBuffers",swapbuffers);
  iuplua_register("IupGLPalette",palette);

  iuplua_regstring(IUP_BUFFER, "IUP_BUFFER");
  iuplua_regstring(IUP_STEREO, "IUP_STEREO");
  iuplua_regstring(IUP_COLOR, "IUP_COLOR");
  iuplua_regstring(IUP_BUFFER_SIZE, "IUP_BUFFER_SIZE");
  iuplua_regstring(IUP_RED_SIZE, "IUP_RED_SIZE");
  iuplua_regstring(IUP_GREEN_SIZE, "IUP_GREEN_SIZE");
  iuplua_regstring(IUP_BLUE_SIZE, "IUP_BLUE_SIZE");
  iuplua_regstring(IUP_ALPHA_SIZE, "IUP_ALPHA_SIZE");
  iuplua_regstring(IUP_DEPTH_SIZE, "IUP_DEPTH_SIZE");
  iuplua_regstring(IUP_STENCIL_SIZE, "IUP_STENCIL_SIZE");
  iuplua_regstring(IUP_ACCUM_RED_SIZE, "IUP_ACCUM_RED_SIZE");
  iuplua_regstring(IUP_ACCUM_GREEN_SIZE, "IUP_ACCUM_GREEN_SIZE");
  iuplua_regstring(IUP_ACCUM_BLUE_SIZE, "IUP_ACCUM_BLUE_SIZE");
  iuplua_regstring(IUP_ACCUM_ALPHA_SIZE, "IUP_ACCUM_ALPHA_SIZE");
  iuplua_regstring(IUP_DOUBLE, "IUP_DOUBLE");
  iuplua_regstring(IUP_SINGLE, "IUP_SINGLE");
  iuplua_regstring(IUP_INDEX, "IUP_INDEX");
  iuplua_regstring(IUP_RGBA, "IUP_RGBA");
  iuplua_regstring(IUP_YES, "IUP_YES");
  iuplua_regstring(IUP_NO, "IUP_NO");

  return lua_dostring("IUPGLCANVAS = {parent = IUPCANVAS}\n"
                      "function IUPGLCANVAS:CreateIUPelement (obj)\n"
                      "  return iupCreateGLCanvas ()\n"
                      "end\n"
                      "function iupglcanvas (o)\n"
                      "  return IUPGLCANVAS:Constructor (o)\n"
                      "end\n"
                      "iup.glcanvas = iupglcanvas\n"
                      "iup_callbacks.action.glcanvas = iup_action_canvas\n");
}
