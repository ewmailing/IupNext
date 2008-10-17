#include <stdlib.h>     /* exit */
#include <string.h>     /* strcpy */
#include <stdio.h>      /* printf */

#include "iup.h"

int newload (Ihandle *h)
{
 char *status;
 char *file_name=NULL;
 char *erro;

 IupPopup (IupGetHandle("escolhe_arq"), IUP_CENTER, IUP_CENTER);
 status = IupGetAttribute(IupGetHandle("escolhe_arq"), IUP_STATUS);

 if(!strcmp (status, "0") )
 {
   file_name = IupGetAttribute(IupGetHandle("escolhe_arq"), IUP_VALUE);
   if (file_name)
   {
      erro = IupLoad (file_name);
      if (!erro)
         IupMessage ("Tudo Bem", "Carregou o LED direito");
        else
         IupMessage ("Error", "Não Carregou o LED direito");
   }
 }
 return IUP_DEFAULT;
}

int quit (void)
{
 return IUP_CLOSE;
}

int loadteste (void)
{
 char *erro;

 erro = IupLoad ("a.led");
 if (!erro)
   IupMessage ("Tudo Bem", "Carregou o LED direito");
 else
   IupMessage ("Error", "Não Carregou o LED direito");

  return IUP_CLOSE;
}

void main (int argc, char *argv[])
{
 char *erro;

 IupOpen(&argc, &argv);
 erro=IupLoad ("..\\tests\\iupfiledlg\\filedlg.led");

 if (erro)
 {
  IupMessage ("Erro", erro);
  return;
 }

 IupSetFunction ("newload",(Icallback)newload);
 IupSetFunction ("loadteste",(Icallback)loadteste);
 IupSetFunction ("exit",(Icallback)quit);
 IupShow(IupGetHandle("load"));
 IupMainLoop();
 IupClose();
}
