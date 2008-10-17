/** \file
 * \brief Windows Driver WinMain for the static library
 *
 * See Copyright Notice in iup.h
 * $Id: winmain.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 */

#include <windows.h>
#include <stdlib.h> /* declaration of __argc and __argv */
#include "iup.h"

#ifdef __WATCOMC__     /* force Watcom to link this module, called from IupOpen */
void iupwinMainDummy(void)
{
  return;
}
#else
extern int main(int, char **);
#endif

#ifndef _IUPDLL /* this module is always linked in the makefile, so it must not define WinMain if building the DLL */
int PASCAL WinMain (HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int ncmdshow)
{
  int rc;
  
#ifdef __WATCOMC__
  {
    extern int _argc;
    extern char** _argv;
    rc = IupMain(_argc, _argv);
  }              
#else
  {
    extern int __argc;     /* this seems to work for all the compilers we tested, except Watcom compilers */
    extern char** __argv;
    rc = main(__argc, __argv);
  }
#endif
  return rc;
}
#endif
