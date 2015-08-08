/** \file
 * \brief IupImgLib
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>

#include "iup.h"

#include "iup_str.h"
#include "iup_image.h"

#include "iup_imglib.h"

/* GTK and Win32 share the same library in Windows */
/* GTK and Motif share the same library in UNIX */

void IupImageLibOpen(void)
{
#ifndef IUP_IMGLIB_DUMMY
  int motif=0, win32=0, gtk=0;
  char* driver;

  if (IupGetGlobal("_IUP_IMAGELIB_OPEN"))
    return;

  driver = IupGetGlobal("DRIVER");
  if (iupStrEqualNoCase(driver, "GTK"))
    gtk = 1;
  else if (iupStrEqualNoCase(driver, "Motif") || iupStrEqualNoCase(driver, "Haiku"))
    motif = 1;
  else if (iupStrEqualNoCase(driver, "Win32"))
    win32 = 1;

  IupSetGlobal("_IUP_IMAGELIB_OPEN", "1");

  /**************** Bitmaps *****************/

#ifndef WIN32
  if (motif)
    iupImglibBaseLibMot16x16Open();
#endif

#ifdef WIN32
  if (win32)
    iupImglibBaseLibWin16x16Open();
#endif  

  if (gtk)
    iupImglibBaseLibGtk16x16Open();

  /***************** Icons *****************/

#ifndef WIN32
  if (motif)
    iupImglibLogosMot32x32Open();
  else
#endif
    iupImglibLogos32x32Open();

  /***************** Logos *****************/

#ifdef IUP_IMGLIB_LARGE
#ifndef WIN32
  if (motif)
    iupImglibLogosMot48x48Open();
  else
#endif
    iupImglibLogos48x48Open();

#ifdef WIN32
  if (win32)
    iupImglibIconsWin48x48Open();
#endif  

  if (gtk)
    iupImglibIconsGtk48x48Open();
#endif  
#endif  
}
 
