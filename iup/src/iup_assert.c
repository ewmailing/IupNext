/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in iup.h
 */

 
#include <string.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <limits.h>  
#include <stdarg.h>

#include "iup.h"

#include "iup_assert.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"


void iupError(const char* format, ...)
{
  static char msg[SHRT_MAX];
  va_list arglist;
  va_start(arglist, format);
  vsprintf(msg, format, arglist);
  va_end(arglist);
#if IUP_ASSERT == CONSOLE 
  fprintf(stderr, msg);
#else
  iupStrMessageShowError(NULL, msg);
#endif
}

void iupAssert(const char* expr, const char* file, int line, const char* func)
{
  if (func)
    iupError("File: %s\n"
             "Line: %d\n"
             "Function: %s\n"
             "Assertive: (%s)", 
             file, line, func, expr);
  else
    iupError("File: %s\n"
             "Line: %d\n"
             "Assertive: (%s)", 
             file, line, expr);
}
