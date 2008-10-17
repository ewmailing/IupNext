/** \file
 * \brief pre-defined dialogs
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>

#include "iup.h" 

#include "iup_predial.h"
#include "iup_str.h"
#include "iup_strmessage.h"
               

static int CB_button_OK (Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "STATUS", "1");
  return IUP_CLOSE;
}

static int CB_button_CANCEL (Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "STATUS", "-1");
  return IUP_CLOSE;
}

static int CB_lista (Ihandle *h, char *n, int o, int v)
{
  static clock_t oldtimesel = 0;
  static int oldopc = 0;
  (void)n; /* not used */
  if (v)
  {
    clock_t timesel = clock();

    IupSetfAttribute(IupGetDialog(h), "_LIST_NUMBER", "%d", o-1);

    if (((timesel-oldtimesel) < 500) && (o == oldopc))
      return IUP_CLOSE;

    oldtimesel = timesel;
    oldopc     = o;
  }
  return IUP_DEFAULT;
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
    IupSetAttribute(listaiup,op,(char*)lista[i]);
  }
  sprintf(lastop,"%d",tam+1);
  IupSetAttribute(listaiup,lastop,NULL);

  ok = IupButton("OK", NULL);
  IupSetAttribute (ok   ,"SIZE" ,"50x");
  IupSetCallback (ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle( "IupListOkButton", ok );

  cancel = IupButton(iupStrMessageGet("IUP_CANCEL"), NULL);
  IupSetAttribute (cancel,"SIZE" ,"50x");
  IupSetCallback (cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle( "IupListCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),  // TODO: this is useless
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");

  dlg_box = IupVbox(
    listaiup,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);

  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","5");

  dialog = IupDialog (dlg_box);

  if (tipo == 1)
  {
    if (opcao<1 || opcao>tam) opcao=1;
    IupSetfAttribute(dialog, "_LIST_NUMBER", "%d", opcao-1);
    sprintf(opcaoini,"%d",opcao);
    IupSetAttribute(listaiup,"VALUE",opcaoini);
    IupSetCallback (listaiup, "ACTION", (Icallback)CB_lista);
  }
  else if ((tipo == 2) && (marcas != NULL))
  {
    m=(char *)marcas;
    for (i=0;i<tam;i++)
      m[i] = marcas[i] ? '+' : '-';
    m[i]='\0';
    IupSetAttribute(listaiup,"MULTIPLE","YES");
    IupSetAttribute(listaiup,"VALUE",m);
  }

  sprintf (maxsize,"%dx%d",max_col*5,max_lin==1?16:max_lin*9+4);
  IupSetAttribute (listaiup,"SIZE" , maxsize);

  IupSetAttribute(dialog,"TITLE",(char*)titulo);
  IupSetAttribute(dialog,"MINBOX","NO");
  IupSetAttribute(dialog,"MAXBOX","NO");
  IupSetAttribute(dialog,"RESIZE","NO");
  IupSetAttribute(dialog,"DEFAULTENTER","IupListOkButton");
  IupSetAttribute(dialog,"DEFAULTESC","IupListCancelButton");
  IupSetAttribute(dialog,"PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dialog,"ICON", IupGetGlobal("ICON"));

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  if ((tipo == 2) && (marcas != NULL))
  {
    m=IupGetAttribute(listaiup,"VALUE");
    for (i=0;i<tam;i++)
      marcas[i] = (m[i] == '+');
  }

  bt = IupGetInt(dialog, "STATUS");
  if (bt != -1 && tipo == 1)
    bt = IupGetInt(dialog, "_LIST_NUMBER");

  IupDestroy(dialog);

  return bt;
}

static int CB_button(Ihandle *ih)
{
  IupSetAttribute(IupGetDialog(ih), "_BUTTON_NUMBER", IupGetAttribute(ih, "_BUTTON_NUMBER"));
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
  IupSetAttribute(lbl_box,"MARGIN","0x0");
  IupSetAttribute(lbl_box,"GAP","0");
  IupSetAttribute(lbl_box,"ALIGNMENT", "ACENTER");

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
  IupSetAttribute(button, "PADDING", "3x3");
  IupAppend(botoes, IupFill());
  IupAppend(botoes, button);
  IupSetCallback (button, "ACTION", (Icallback)CB_button);
  default_esc = "IupAlarmButton1";

  if (b2 != NULL)
  {
    button = IupButton(b2, NULL);
    IupSetHandle( "IupAlarmButton2", button );
    IupSetAttribute(button, "_BUTTON_NUMBER", "2");
    IupSetAttribute(button, "PADDING", "3x3");
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
    IupSetAttribute(button, "PADDING", "3x3");
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

  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog,"TITLE",(char*)(title ? title : ""));
  IupSetAttribute(dialog,"MINBOX" ,"NO");
  IupSetAttribute(dialog,"MAXBOX" ,"NO");
  IupSetAttribute(dialog,"RESIZE" ,"NO");
  IupSetAttribute(dialog,"DEFAULTENTER","IupAlarmButton1");
  IupSetAttribute(dialog,"DEFAULTESC",default_esc);
  IupSetAttribute(dialog,"PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dialog,"ICON", IupGetGlobal("ICON"));

  IupMap(dialog);

  wb = IupGetInt(IupGetHandle("IupAlarmButton1"), "SIZE");

  if (b2 != NULL)
  {
    int wb2 = IupGetInt(IupGetHandle("IupAlarmButton2"), "SIZE");
    if (wb2 > wb) wb = wb2;
  }
  if (b3 != NULL)
  {
    int wb3 = IupGetInt(IupGetHandle("IupAlarmButton3"), "SIZE");
    if (wb3 > wb) wb = wb3;
  }

  sprintf(button_size, "%dx", wb);

  IupSetAttribute(IupGetHandle("IupAlarmButton1"), "SIZE", button_size);
  IupSetAttribute(IupGetHandle("IupAlarmButton2"), "SIZE", button_size);
  IupSetAttribute(IupGetHandle("IupAlarmButton3"), "SIZE", button_size);

  /* Force to calculate dialog's size again. */
  IupSetAttribute(dialog, "RASTERSIZE", NULL);
  IupRefresh(dialog);

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  bt = IupGetInt(dialog, "_BUTTON_NUMBER");

  IupDestroy(dialog);
  free(aux);

  return bt;
}

int  iupDataEntry  (int    maxlin,
                    int    maxcol[],
                    int    maxscr[],
                    char  *title,
                    char  *text[],
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
    IupSetAttribute(txt[i],"VALUE",data[i]);
    IupStoreAttribute(txt[i],"SIZE",scroll);
    IupStoreAttribute(txt[i],"NC",sizecol);

    hb = IupHbox(IupLabel(text[i]), IupFill(), txt[i],NULL);
    IupSetAttribute(hb,"MARGIN","0x0");
    IupSetAttribute(hb,"ALIGNMENT","ACENTER");
    IupAppend(vb, hb);
  }

  ok = IupButton("OK", NULL);
  IupSetAttribute(ok, "SIZE", "50x");
  IupSetCallback(ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle("iupDataEntryOkButton", ok);

  cancel = IupButton(iupStrMessageGet("IUP_CANCEL"), NULL);
  IupSetAttribute(cancel, "SIZE", "50x");
  IupSetCallback(cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle("iupDataEntryCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");

  dlg_box = IupVbox(
    IupFrame(vb),
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);
  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog,"TITLE",title);
  IupSetAttribute(dialog,"MINBOX","NO");
  IupSetAttribute(dialog,"MAXBOX","NO");
  IupSetAttribute(dialog,"RESIZE","NO");
  IupSetAttribute(dialog,"DEFAULTENTER","iupDataEntryOkButton");
  IupSetAttribute(dialog,"DEFAULTESC","iupDataEntryCancelButton");
  IupSetAttribute(dialog,"PARENTDIALOG",IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dialog,"ICON", IupGetGlobal("ICON"));

  IupPopup(dialog,IUP_CENTER,IUP_CENTER);

  for (i=0; i<maxlin; i++)
  {
    data[i] = (char *)iupStrDup(IupGetAttribute(txt[i], "VALUE"));
  }

  free(txt);

  bt = IupGetInt(dialog, "STATUS");
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
  for (i=n-1;i>=0; i--)
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

  IupSetAttribute(gf, "FILTER", filter);
  IupSetAttribute(gf, "DIRECTORY", dir);
  IupSetAttribute(gf, "ALLOWNEW", "YES");
  IupSetAttribute(gf, "NOCHANGEDIR", "YES");
  IupSetAttribute(gf, "PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(gf, "ICON", IupGetGlobal("ICON"));

  IupPopup( gf, IUP_CENTER, IUP_CENTER );

  value = IupGetAttribute( gf, "VALUE" );
  if (value) strcpy( file, value );
  ret = IupGetInt( gf, "STATUS" );

  IupDestroy(gf);

  return ret;
}

int IupGetText(const char* title, char* text)
{
  Ihandle *ok, *cancel, *multi_text, *button_box, *dlg_box, *dialog;
  int bt;

  multi_text = IupMultiLine("do_nothing");
  IupSetAttribute(multi_text,"EXPAND", "YES");
  IupSetAttribute(multi_text,"SIZE", "200x80");
  IupSetAttribute(multi_text,"VALUE", text);
  IupSetAttribute(multi_text,"FONT", "Courier, 12");

  ok   = IupButton("OK", NULL);
  IupSetAttribute (ok   ,"SIZE" ,"50x");
  IupSetCallback(ok, "ACTION", (Icallback)CB_button_OK);
  IupSetHandle( "IupGetTextOkButton", ok );

  cancel  = IupButton(iupStrMessageGet("IUP_CANCEL"), NULL);
  IupSetAttribute (cancel,"SIZE" ,"50x");
  IupSetCallback(cancel, "ACTION", (Icallback)CB_button_CANCEL);
  IupSetHandle( "IupGetTextCancelButton", cancel );

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    ok,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    cancel,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");

  dlg_box = IupVbox(
    multi_text,
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);

  IupSetAttribute(dlg_box,"MARGIN","10x10");
  IupSetAttribute(dlg_box,"GAP","5");

  dialog = IupDialog (dlg_box);

  IupSetAttribute (dialog,"TITLE",(char*)title);
  IupSetAttribute (dialog,"MINBOX","NO");
  IupSetAttribute (dialog,"MAXBOX","NO");
  IupSetAttribute (dialog,"DEFAULTENTER","IupGetTextOkButton");
  IupSetAttribute (dialog,"DEFAULTESC","IupGetTextCancelButton");
  IupSetAttribute (dialog,"PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute (dialog,"ICON", IupGetGlobal("ICON"));

  IupPopup(dialog, IUP_CENTER, IUP_CENTER);

  bt = IupGetInt(dialog, "STATUS");
  if (bt==1)
    strcpy(text, IupGetAttribute(multi_text, "VALUE"));

  IupDestroy(dialog);
  return bt;
}

void iupVersionDlg(void)
{
   Ihandle* dial, *ok;

   dial = IupDialog(IupVbox(IupFrame(IupVbox(
                        IupLabel(IUP_VERSION),
                        IupLabel(IUP_VERSION_DATE),
                        IupLabel(IUP_COPYRIGHT),
                        NULL)), 
                      ok = IupButton("Ok", NULL),
                      NULL));

   IupSetCallback(ok, "ACTION", (Icallback)CB_button_OK);

   IupSetAttribute(dial,"TITLE","IUP Version");
   IupSetAttribute(dial,"MENUBOX","NO");
   IupSetAttribute(dial,"MINBOX","NO");
   IupSetAttribute(dial,"MAXBOX","NO");
   IupSetAttribute(dial,"RESIZE","NO");

   IupSetAttribute(dial,"GAP","5");
   IupSetAttribute(dial,"MARGIN","5");

   IupPopup(dial, IUP_CENTER, IUP_CENTER);
   IupDestroy(dial);
}
