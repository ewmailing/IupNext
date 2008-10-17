#include <stdio.h>
#include "iup.h"

Ihandle *dlg, *mensagens, *bt, *out;

int add( void )
{
  char *value = IupGetAttribute(out, IUP_VALUE);
  IupSetAttribute(mensagens, IUP_APPEND, value);
  IupSetAttribute(out, IUP_VALUE, "");
  return IUP_DEFAULT ;
}

void setcallbacks( void )
{
  IupSetFunction( "add", (Icallback) add ) ;
}

int main(int argc, char **argv)
{
  if ( IupOpen(&argc, &argv) == IUP_ERROR )
  {
    fprintf( stderr, "Erro na abertura do IUP.") ;
    return 0 ;
  }

  IupSetLanguage( IUP_PORTUGUESE ) ;

  mensagens = IupMultiLine("");
  out = IupMultiLine("");
  bt = IupButton("Add line", "add");
  IupSetAttribute(mensagens, IUP_SIZE, "200x200");
  IupSetAttribute(out, IUP_SIZE, "200x200");
  dlg = IupDialog(IupVbox(mensagens, out, bt, NULL));

  IupShow(dlg);
 
  setcallbacks() ;
  IupMainLoop() ;
 
  IupClose() ;

  return 1 ;
}
