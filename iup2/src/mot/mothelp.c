/** \file
 * \brief Motif Driver IupHelp
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "iup.h"

#ifdef LINUX
#define IHELPAPP "firefox"
#else
#define IHELPAPP "netscape"
#endif

int IupHelp(const char *url)
{
	char *cmd;
  int ret;
	char *browser = getenv("IUP_HELPAPP");
	if (!browser) browser = IupGetGlobal("HELPAPP");
	if (!browser) browser = IHELPAPP;
  cmd = (char*)malloc(sizeof(char)*(strlen(url)+strlen(browser)+3));
  sprintf( cmd, "%s %s &", browser, url);
  ret = system(cmd); 
  free(cmd);
  if (ret == -1)
    return -1;
  return 1;
}
