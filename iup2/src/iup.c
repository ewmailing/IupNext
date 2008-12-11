/** \file
 * \brief miscelaneous functions
 *
 * See Copyright Notice in iup.h
 * $Id: iup.c,v 1.3 2008-12-11 19:08:05 scuri Exp $
 */

/*! \mainpage IUP<br>Portable Graphical User Interface Library
 *
 * \section intro Introduction
 *
 * Internal documentation of the IUP library.
 *
 * \section author Author
 *
 * Basic Software Group @ Tecgraf/PUC-Rio
 * - Antonio Scuri scuri@tecgraf.puc-rio.br
 * - Mark Glasberg mark@tecgraf.puc-rio.br
 *
 * \section codestd Code Standards
 *
 * \subsection func Functions (title case format)
 *  - Iup - User API, implemented in the core
 *  - iup - Internal Core API, implemented in the core, used in the core, in Win or in Mot
 *  - iupwin - Windows Internal API, implemented in Win, used in Win
 *  - iupmot - Motif Internal API, implemented in Mot, used in Mot
 *  - iupdrv - Driver API, implemented in Win or Mot, used in the core
 *  - iupcpi - Control API, implemented in the core, used in the controls, in Win or in Mot
 *  - winXxx - Windows local functions, Xxx is module name
 *  - motXxx - Motif local functions, Xxx is module name
 *  - ctrlXxx - Control "ctrl" local functions, Xxx is module name if necessary
 * 
 * \subsection glob Globais Variables (lower case format)
 *  - iup_
 *  - iupwin_
 *  - iupmot_
 *                    
 * \subsection loc Local Variables (lower case format, using module name)
 *  - ixxx_
 *                    
 * \subsection fil Files
 *  - iupxxx.h - public
 *  - ixxx.h/c - core
 *  - winxxx.h/c - windows
 *  - motxxx.h/c - motif    
 *   
 * \subsection strc Structures
 *  - Ixxxx
 *   
 * \subsection com File Comments (at start)
 *  - Check an existant file for example.
 *    
 * \subsection inc Include Defines
 *  -  __XXX_H
 *
 */

#include <stdlib.h>  /* NULL */

#include "iup.h"

/* This appears only here to avoid changing the iup.h header fo bug fixes */
#define IUP_VERSION_FIX ".1"
#define IUP_VERSION_FIX_NUMBER 1

const char iup_ident[] = 
  "$IUP: " IUP_VERSION IUP_VERSION_FIX " " IUP_COPYRIGHT " $\n"
  "$URL: www.tecgraf.puc-rio.br/iup $\n";

/* Using this, if you look for the string TECVER, you will find also the library version. */
const char *iup_tecver = "TECVERID.str:Iup:LIB:" IUP_VERSION IUP_VERSION_FIX;

char* IupVersion(void)
{
  (void)iup_tecver;
  (void)iup_ident;
  return IUP_VERSION IUP_VERSION_FIX;
}

char* IupVersionDate(void)
{
  return IUP_VERSION_DATE;
}
 
int IupVersionNumber(void)
{
  return IUP_VERSION_NUMBER+IUP_VERSION_FIX_NUMBER;
}

/***********************************************************/

static const char *iup_language = NULL;

void IupSetLanguage(const char *lng)
{
  iup_language = lng;
}

char *IupGetLanguage(void)
{
  return (char*)iup_language;
}
