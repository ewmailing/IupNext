/*
IupGetAttribute/Int/Float Example 
Uses IupGetAttribute/IupGetInt and IupGetFloat to print a number entered by the user. The printing will occur when the user exits the program. The number will be printed as an integer, a floating point and a text.
*/

#include <string.h>  /* strchr */
#include <ctype.h>  /* iscntrl */

#include "iup.h"
#include "iupkey.h"

int acao_numero (Ihandle *self, int c)
{
  static char caracteres_validos[]="0123456789.+-Ee";

  if (c == K_CR)  /* replaces TAB for ENTER */
    return K_TAB;
  else if (strchr (caracteres_validos, c))  /* c is a valid character */
    return IUP_DEFAULT;
  else if (iscntrl(c))  /* c is a control character (TAB, BACKSPACE, ...) */
    return IUP_DEFAULT;

  return IUP_IGNORE;
}
void main(int argc, char **argv)
{
  Ihandle *campo;    /* TEXT interface element */
  Ihandle *mensagem; /* LABEL interface element */
  Ihandle *prompt;   /* box containing the message and the field */
  Ihandle* dlg;

  IupOpen(&argc, &argv);  /* initializes IUP */

  campo = IupText("acao_numero");                           /* creates TEXT field */
  IupSetFunction ("acao_numero", (Icallback)acao_numero);   /* registers callback */
  IupSetAttribute (campo, "VALUE", "1.0");                /* defines initial value */

  mensagem = IupLabel("Entre com um número: "); /* creates message */
  prompt = IupHbox(mensagem, campo, NULL);  /* creates prompt */

  dlg = IupDialog(prompt)
  IupShow(dlg); /* displays and creates dialog */
  IupMainLoop ();               /* interacts with user */

  printf ("Número Inteiro: %d", IupGetInt (campo, "VALUE"));   /* prints the integer number entered by the user */
  printf ("Número Real: %g", IupGetFloat (campo, "VALUE"));       /* prints the real number entered by the user */
  printf ("Texto Digitado: %s", IupGetAttribute(campo, "VALUE"));  /* prints the text entered by the user */

  IupDestroy(dlg);
  IupClose ();  /* ends IUP */  
}
