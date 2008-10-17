/** \file
 * \brief iupgc pre-defined dialog
 * language control
 *
 * See Copyright Notice in iup.h
 * $Id: igc_lng.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iup.h>
#include <iupcpi.h>
#include "igc_lng.h"
#include "istrutil.h"

#define NUM_MAX_TEXT 9

static struct
{
  char text[10];
  char pt[50];
  char in[70];
}
IupGCText[NUM_MAX_TEXT] =
{
  {"red",        "Verm",            "Red"               },
  {"green",      "Verde",           "Green"             },
  {"blue",       "Azul",            "Blue"              },
  {"hue",        "Matiz",           "Hue"               },
  {"lum",        "Lum",             "Lum"               },
  {"ok",         "Ok",              "Ok"                },
  {"sat",        "Sat",             "Sat"               },
  {"cancel",     "Cancelar",        "Cancel"            },
  {"colorsel",   "Seleção de cor",  "Color selection"   }
};


char *iupgcGetText(char *v)
{
  char* type = NULL;
  int c = 0;

  type = IupGetLanguage();
  for(c = 0; c < NUM_MAX_TEXT; c++)
  {
    if(iupStrEqual(v, IupGCText[c].text))
    {
      /* If defined, but diferent from NO use English. */
      if(iupStrEqual(type, IUP_ENGLISH))
        return IupGCText[c].in;
      else
        return IupGCText[c].pt;
    }
  }

  assert(0); /* Should always return something */
  return NULL; /* font was not an IUP font */

}
