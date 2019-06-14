/** \file
 * \brief Binding of iupwebbrowser to Lua.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPLUAWEB_H 
#define __IUPLUAWEB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */
#ifndef IUPLUAWEB_EXPORT
#ifdef IUPLUAWEB_BUILD_LIBRARY
  #ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define IUPLUAWEB_EXPORT EMSCRIPTEN_KEEPALIVE
  #elif WIN32
    #define IUPLUAWEB_EXPORT __declspec(dllexport)
  #elif defined(__GNUC__) && __GNUC__ >= 4
    #define IUPLUAWEB_EXPORT __attribute__ ((visibility("default")))
  #else
    #define IUPLUAWEB_EXPORT
  #endif
#else
  #define IUPLUAWEB_EXPORT
#endif /* IUPLUAWEB_BUILD_LIBRARY */
#endif /* IUPLUAWEB_EXPORT */
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


IUPLUAWEB_EXPORT int iupweblua_open (lua_State * L);

#ifdef __cplusplus
}
#endif

#endif
