/** \file
 * \brief pre-defined dialogs
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>

#include "iup.h"
#include "ipredial.h"
#include "istrutil.h"

#define strok     (iupStrEqual(IupGetLanguage(), IUP_ENGLISH) ? "OK"     : "Confirma")
#define strcancel (iupStrEqual(IupGetLanguage(), IUP_ENGLISH) ? "Cancel" : "Cancela")

static int CB_button_OK (Ihandle* self)
{
  IupSetAttribute(IupGetDialog(self), IUP_STATUS, "1");
  return IUP_CLOSE;
}

static int CB_button_CANCEL (Ihandle* self)
{
  IupSetAttribute(IupGetDialog(self), IUP_STATUS, "-1");
  return IUP_CLOSE;
}

static int CB_lista (Ihandle *h, char *n, int o, int v)
{
  static clock_t timesel, oldtimesel = 0;
  static int oldopc = 0;
  (void)n; /* not used */
  if( v )
  {
    IupSetfAttribute(IupGetDialog(h), "_LIST_NUMBER", "%d", o-1);
    timesel = clock();
    if(((timesel-oldtimesel) < 500) && (o == oldopc))
    {
      return IUP_CLOSE;
    }
    oldtimesel = timesel;
    oldopc     = o;
  }
  return IUP_DEFAULT;
}

void IupMessagef(const char *title, const char *f, ...)
{
  static char m[SHRT_MAX];
  va_list arglist;
  va_start(arglist, f);
  vsprintf(m, f, arglist);
  va_end (arglist);
  IupMessage(title, m);
}

int IupListDialog (int tipo, const char *titulo, int tam, const char *lista[],
                   int opcao, int max_col, int max_lin, int marcas[])
{
  Ihandle *listaiup, *ok, *dialog, *cancel, *dlg_box, *button_box;
  int i, bt;
  char op[5];
  char lastop[5];
  char maxsize[20];
  char opcaoini[20];
  char *m=NULL;

  if (tam > 999)
    tam = 999;

  listaiup = IupList(NULL);

  for (i=0;i<tam;i++)
  {
    sprintf(op,"%d",i+1);
    IupSetAttribute(listaiup,op,lista[i]);
  }
  sprintf(lastop,"%d",tam+1);
  IupSetAttribute(listaiup,lastop,NULL);

  ok = IupButton(strok, NULL);
  IupSetAttribute (ok   ,IUP_SIZE ,"50x");
  IupSetCallback (ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle( "IupListOkButton", ok );

  cancel = IupButton(strcancel, NULL);
  IupSetAttribute (cancel,IUP_SIZE ,"50x");
  IupSetCallback (cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle( "IupListCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,IUP_MARGIN,"0x0");

  dlg_box = IupVbox(
    listaiup,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);

  IupSetAttribute(dlg_box,IUP_MARGIN,"10x10");
  IupSetAttribute(dlg_box,IUP_GAP,"5");

  dialog = IupDialog (dlg_box);

  if (tipo == 1)
  {
    if (opcao<1 || opcao>tam) opcao=1;
    IupSetfAttribute(dialog, "_LIST_NUMBER", "%d", opcao-1);
    sprintf(opcaoini,"%d",opcao);
    IupSetAttribute(listaiup,IUP_VALUE,opcaoini);
    IupSetCallback (listaiup, "ACTION", (Icallback)CB_lista);
  }
  else if ((tipo == 2) && (marcas != NULL))
  {
    m=(char *)marcas;
    for (i=0;i<tam;i++)
      m[i] = marcas[i] ? '+' : '-';
    m[i]='\0';
    IupSetAttribute(listaiup,IUP_MULTIPLE,IUP_YES);
    IupSetAttribute(listaiup,IUP_VALUE,m);
  }

  sprintf (maxsize,"%dx%d",max_col*5,max_lin==1?16:max_lin*9+4);
  IupSetAttribute (listaiup,IUP_SIZE , maxsize);

  IupSetAttribute (dialog,IUP_TITLE,titulo);
  IupSetAttribute (dialog,IUP_MINBOX,IUP_NO);
  IupSetAttribute (dialog,IUP_MAXBOX,IUP_NO);
  IupSetAttribute (dialog,IUP_RESIZE,IUP_NO);
  IupSetAttribute (dialog,IUP_DEFAULTENTER,"IupListOkButton");
  IupSetAttribute (dialog,IUP_DEFAULTESC,"IupListCancelButton");
  IupSetAttribute (dialog, IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute (dialog, IUP_ICON, IupGetGlobal(IUP_ICON));

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  if ((tipo == 2) && (marcas != NULL))
  {
    m=IupGetAttribute(listaiup,IUP_VALUE);
    for (i=0;i<tam;i++)
      marcas[i] = (m[i] == '+');
  }

  bt = IupGetInt(dialog, IUP_STATUS);
  if (bt != -1 && tipo == 1)
    bt = IupGetInt(dialog, "_LIST_NUMBER");

  IupDestroy(dialog);

  return bt;
}

static int CB_button(Ihandle *self)
{
  IupSetAttribute(IupGetDialog(self), "_BUTTON_NUMBER", IupGetAttribute(self, "_BUTTON_NUMBER"));
  return IUP_CLOSE;
}

int IupAlarm (const char *title, const char *m, const char *b1, const char *b2, const char *b3)
{
  Ihandle  *dialog, *dlg_box, *lbl_box, *botoes, *button;
  char *aux, *linha, *default_esc;
  int i, bt, wb;
  char button_size[10];

  m = m ? m : "";

  if (b1 == NULL)
    return 0;

  aux=(char *)iupStrDup(m);
  if (aux == NULL)
    return 0;

  lbl_box = IupVbox(NULL);
  IupSetAttribute(lbl_box,IUP_MARGIN,"0x0");
  IupSetAttribute(lbl_box,IUP_GAP,"0");
  IupSetAttribute(lbl_box,IUP_ALIGNMENT, IUP_ACENTER);

  for (i=0,linha=aux; linha[i]; ++i)
  {
    if (linha[i] == '\n')
    {
      linha[i] = '\0';
      IupAppend(lbl_box,IupHbox(IupFill(),IupLabel(linha),IupFill(),NULL));
      linha=&linha[i+1];
      i=-1;
    }
  }

  if (*linha)
    IupAppend(lbl_box,IupHbox(IupFill(),IupLabel(linha),IupFill(),NULL));

  botoes = IupHbox(NULL);

  button = IupButton(b1, NULL);
  IupSetHandle("IupAlarmButton1", button );
  IupSetAttribute(button, "_BUTTON_NUMBER", "1");
  IupAppend(botoes, IupFill());
  IupAppend(botoes, button);
  IupSetCallback (button, "ACTION", (Icallback)CB_button);
  default_esc = "IupAlarmButton1";

  if (b2 != NULL)
  {
    button = IupButton(b2, NULL);
    IupSetHandle( "IupAlarmButton2", button );
    IupSetAttribute(button, "_BUTTON_NUMBER", "2");
    IupAppend(botoes, IupFill());
    IupAppend(botoes, button);
    IupSetCallback (button, "ACTION", (Icallback)CB_button);
    default_esc = "IupAlarmButton2";
  }

  if (b3 != NULL)
  {
    button   = IupButton(b3, NULL);
    IupSetHandle( "IupAlarmButton3", button );
    IupSetAttribute(button, "_BUTTON_NUMBER", "3");
    IupAppend(botoes, IupFill());
    IupAppend(botoes, button);
    IupSetCallback (button, "ACTION", (Icallback)CB_button);
    default_esc = "IupAlarmButton3";
  }

  IupAppend(botoes, IupFill());

  dlg_box = IupVbox (
    IupSetAttributes(IupFill(), "SIZE=1x"),
    lbl_box,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    botoes,
    NULL);

  IupSetAttribute(dlg_box,IUP_MARGIN,"10x10");
  IupSetAttribute(dlg_box,IUP_GAP,"5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog,IUP_TITLE,title ? title : "");
  IupSetAttribute(dialog,IUP_MINBOX ,IUP_NO);
  IupSetAttribute(dialog,IUP_MAXBOX ,IUP_NO);
  IupSetAttribute(dialog,IUP_RESIZE ,IUP_NO);
  IupSetAttribute(dialog,IUP_DEFAULTENTER,"IupAlarmButton1");
  IupSetAttribute(dialog,IUP_DEFAULTESC,default_esc);
  IupSetAttribute(dialog, IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute(dialog, IUP_ICON, IupGetGlobal(IUP_ICON));

  IupMap(dialog);

  wb = IupGetInt(IupGetHandle("IupAlarmButton1"), IUP_SIZE);

  if (b2 != NULL)
  {
    int wb2 = IupGetInt(IupGetHandle("IupAlarmButton2"), IUP_SIZE);
    if(wb2 > wb) wb = wb2;
  }
  if (b3 != NULL)
  {
    int wb3 = IupGetInt(IupGetHandle("IupAlarmButton3"), IUP_SIZE);
    if(wb3 > wb) wb = wb3;
  }

  sprintf(button_size, "%dx", wb);

  IupSetAttribute(IupGetHandle("IupAlarmButton1"), IUP_SIZE, button_size);
  IupSetAttribute(IupGetHandle("IupAlarmButton2"), IUP_SIZE, button_size);
  IupSetAttribute(IupGetHandle("IupAlarmButton3"), IUP_SIZE, button_size);

  /* Force to calculate dialog's size again. */
  IupSetAttribute(dialog, IUP_RASTERSIZE, 0);

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  bt = IupGetInt(dialog, "_BUTTON_NUMBER");

  IupDestroy(dialog);
  free(aux);

  return bt;
}

int  iupDataEntry  ( int    maxlin,
                    int    maxcol[],
                    int    maxscr[],
                    const char  *title,
                    const char  **text,
                    char  *data[] )

{
  int i, bt;
  Ihandle *ok, *cancel, *dialog, *vb, *hb, **txt, *button_box, *dlg_box;
  char scroll[20];
  char sizecol[20];

  txt = (Ihandle **) calloc(maxlin, sizeof(Ihandle*));
  if (txt == NULL) return -2;

  vb = IupVbox(NULL);

  for (i=0; i<maxlin; i++)
  {
    sprintf(scroll, "%d", 4*maxscr[i]+4);
    sprintf(sizecol, "%d", maxcol[i]);

    txt[i] = IupText("do_nothing");
    IupSetAttribute(txt[i],IUP_VALUE,data[i]);
    IupStoreAttribute(txt[i],IUP_SIZE,scroll);
    IupStoreAttribute(txt[i],IUP_NC,sizecol);

    hb = IupHbox(IupLabel(text[i]), IupFill(), txt[i],NULL);
    IupSetAttribute(hb,IUP_MARGIN,"0x0");
    IupSetAttribute(hb,IUP_ALIGNMENT,IUP_ACENTER);
    IupAppend(vb, hb);
  }

  ok = IupButton(strok, NULL);
  IupSetAttribute(ok, IUP_SIZE, "50x");
  IupSetCallback(ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle("iupDataEntryOkButton", ok);

  cancel = IupButton(strcancel, NULL);
  IupSetAttribute(cancel, IUP_SIZE, "50x");
  IupSetCallback(cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle("iupDataEntryCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,IUP_MARGIN,"0x0");

  dlg_box = IupVbox(
    IupFrame(vb),
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);
  IupSetAttribute(dlg_box,IUP_MARGIN,"10x10");
  IupSetAttribute(dlg_box,IUP_GAP,"5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog,IUP_TITLE,title);
  IupSetAttribute(dialog,IUP_MINBOX,IUP_NO);
  IupSetAttribute(dialog,IUP_MAXBOX,IUP_NO);
  IupSetAttribute(dialog,IUP_RESIZE,IUP_NO);
  IupSetAttribute(dialog,IUP_DEFAULTENTER,"iupDataEntryOkButton");
  IupSetAttribute(dialog,IUP_DEFAULTESC,"iupDataEntryCancelButton");
  IupSetAttribute(dialog,IUP_PARENTDIALOG,IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute(dialog,IUP_ICON, IupGetGlobal(IUP_ICON));

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  for (i=0; i<maxlin; i++)
  {
    data[i] = (char *)iupStrDup(IupGetAttribute(txt[i], IUP_VALUE));
  }

  free(txt);

  bt = IupGetInt(dialog, IUP_STATUS);
  IupDestroy(dialog);
  return bt;
}

int IupGetFile( char* file )
{
  Ihandle *gf = 0;
  int i,ret,n;
  char *value;
  char filter[4096] = "*.*";
  char dir[4096];

  if (!file) return -3;

  gf = IupFileDlg();
  if (!gf) return -2;

  n = strlen(file);

  /* Look for last folder separator and split filter from directory */
  for(i=n-1;i>=0; i--)
  {
    if (file[i] == '\\' || file[i] == '/') 
    {
      strncpy(dir, file, i+1);
      dir[i+1] = 0;   

      strcpy(filter, file+i+1);
      filter[n-i] = 0;

      break;
    }
  }

  IupSetAttribute(gf, IUP_FILTER, filter);
  IupSetAttribute(gf, IUP_DIRECTORY, dir);
  IupSetAttribute(gf, IUP_ALLOWNEW, IUP_YES );
  IupSetAttribute(gf, IUP_NOCHANGEDIR, IUP_YES);
  IupSetAttribute(gf, IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute(gf, IUP_ICON, IupGetGlobal(IUP_ICON));

  IupPopup( gf, IUP_CENTER, IUP_CENTER );

  value = IupGetAttribute( gf, IUP_VALUE );
  if (value) strcpy( file, value );
  ret = IupGetInt( gf, IUP_STATUS );

  IupDestroy(gf);

  return ret;
}

int IupGetText(const char* title, char* text)
{
  Ihandle *ok, *cancel, *multi_text, *button_box, *dlg_box, *dialog;
  int bt;

  multi_text = IupMultiLine("do_nothing");
  IupSetAttribute(multi_text,IUP_EXPAND, IUP_YES);
  IupSetAttribute(multi_text, IUP_SIZE, "200x80");
  IupSetAttribute(multi_text,IUP_VALUE, text);
  IupSetAttribute(multi_text,IUP_FONT, IUP_COURIER_NORMAL_12);

  ok   = IupButton(strok, NULL);
  IupSetAttribute (ok   ,IUP_SIZE ,"50x");
  IupSetCallback(ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle( "IupGetTextOkButton", ok );

  cancel  = IupButton(strcancel, NULL);
  IupSetAttribute (cancel,IUP_SIZE ,"50x");
  IupSetCallback(cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle( "IupGetTextCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,IUP_MARGIN,"0x0");

  dlg_box = IupVbox(
    multi_text,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);

  IupSetAttribute(dlg_box,IUP_MARGIN,"10x10");
  IupSetAttribute(dlg_box,IUP_GAP,"5");

  dialog = IupDialog (dlg_box);

  IupSetAttribute (dialog,IUP_TITLE,title);
  IupSetAttribute (dialog,IUP_MINBOX,IUP_NO);
  IupSetAttribute (dialog,IUP_MAXBOX,IUP_NO);
  IupSetAttribute (dialog,IUP_DEFAULTENTER,"IupGetTextOkButton");
  IupSetAttribute (dialog,IUP_DEFAULTESC,"IupGetTextCancelButton");
  IupSetAttribute (dialog,IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute (dialog, IUP_ICON, IupGetGlobal(IUP_ICON));

  IupPopup(dialog, IUP_CENTER, IUP_CENTER);

  bt = IupGetInt(dialog, IUP_STATUS);
  if (bt==1)
    strcpy(text, IupGetAttribute(multi_text, IUP_VALUE));
  else
    bt = 0;  /* return 0 instead of -1 */

  IupDestroy(dialog);
  return bt;
}
