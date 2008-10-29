/** \file
 * \brief Motif Driver IupFileDlg
 *
 * See Copyright Notice in iup.h
 * $Id: motfiledlg.c,v 1.2 2008-10-29 19:27:01 scuri Exp $
 */

#if defined(__STDC__) && defined(ULTRIX)
#define X$GBLS(x)  #x
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#ifdef _VMS_
#include <unixlib.h>
#endif
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include <Xm/Xm.h>

#include "iglobal.h"
#include "icpi.h"
#include "idrv.h"
#include "motif.h"

#define DIR_SEPARATOR '/'

/* -------------------------------------------------- */

static void getStrFromXmstr (XmString xmstr, char *str)
{
  char *tmp;
  if (str == NULL) return;
  XmStringGetLtoR(xmstr, XmSTRING_DEFAULT_CHARSET, &tmp);
  strcpy(str, tmp);
  XtFree(tmp);
}

/* extrai um texto (char *) de uma resource (XmString).
O ponteiro str deve apontar para uma area ja' alocada
*/
static void getStrFromResource (Widget w, char *resource, char *str)
{
  XmString xmstr;
  if (str == NULL) return;
  XtVaGetValues(w, resource, &xmstr, NULL);
  getStrFromXmstr(xmstr, str);
}

static char *strrtok (char *str, char token)
{
  char *tmp = strrchr(str, token);
  if (tmp == NULL) return NULL;
  return &tmp[1];
}

static void motFileDlgFileCb(Widget w, XtPointer client_data, XtPointer call_data)
{
  char file[256];
  struct stat status;
  XmFileSelectionBoxCallbackStruct* cback = \
    (XmFileSelectionBoxCallbackStruct*)call_data;
  int reason = cback->reason;
  Ihandle* h = (Ihandle*)client_data;
  int retcode = -1;
  char dir[256];

  if (reason != XmCR_CANCEL)
  {
    char *str = XmTextGetString(XmFileSelectionBoxGetChild(w, XmDIALOG_TEXT));
    char *dtype = iupGetEnv(h, IUP_DIALOGTYPE);
    int isSaveDlg = (dtype && iupStrEqualNoCase(dtype, IUP_SAVE));
    char* language = IupGetLanguage();
    int chooseDir = iupCheck(h, "_IUPMOT_FILEDLG_DIRONLY_") == YES;

    getStrFromXmstr(cback->value, file);
    if (str[0] != DIR_SEPARATOR)
    {
      getStrFromResource(w, XmNdirectory, dir);
      strcat(dir, str);
    }
    else
    {
      strcpy(dir, str);
    }
    IupStoreAttribute(h, IUP_VALUE, dir);

    if (stat(dir, &status) == -1 && errno == ENOENT && !chooseDir)
    { /* e' um arquivo novo */
      int novo = YES;

      if(iupCheck(h, IUP_ALLOWNEW) == NO)
        novo = NO ; 
      else if(iupCheck(h, IUP_ALLOWNEW) == YES)
        novo = YES ;
      else
      {
        if(iupGetEnv(h, IUP_DIALOGTYPE) == NULL)
          novo = NO ;    
        else
        {
          if(iupStrEqual(iupGetEnv(h, IUP_DIALOGTYPE),IUP_OPEN))
            novo = NO;
        }
      }

      if (novo == NO)
      {
        if (iupStrEqual(language, IUP_ENGLISH))
          IupMessage("Error", "File does not exist.");
        else
          IupMessage("Erro", "Arquivo inexistente.");

        XtFree(str);
        return;
      }
      retcode = 1;
    }
    /* nao pode diretorio, mas foi escolhido um */
    else if (!chooseDir && S_ISDIR(status.st_mode))
    { /* trata-se de um _diretorio_ */
      if (iupStrEqual(language, IUP_ENGLISH))
        IupMessage("Error", "The specified file is a directory.");
      else
        IupMessage("Erro", "Arquivo especificado é um diretório.");

      XtFree(str);
      return;
    }
    /* tem que ser diretorio, mas nao foi escolhido um */
    else if (chooseDir && !S_ISDIR(status.st_mode))
    { /* trata-se de um _arquivo_ */
      if (iupStrEqual(language, IUP_ENGLISH))
        IupMessage("Error", "The specified file is not a directory.");
      else
        IupMessage("Erro", "Arquivo especificado nao é um diretório.");

      XtFree(str);
      return;
    }
    else if (! S_ISDIR(status.st_mode))
    { /* trata-se de um _arquivo_ ja' existente */
      if (isSaveDlg)
      {
        int ret = 1;

        if (iupCheck(h, IUP_NOOVERWRITEPROMPT)!=YES)
        {
          if (iupStrEqual(language, IUP_ENGLISH))
            ret = IupAlarm("Confirmation", "Overwrite existing file?", "Yes", "No", NULL);
          else
            ret = IupAlarm("Confirmação", "Sobrescrever arquivo?", "Sim", "Não", NULL);
        }

        if (ret == 2)
        {
          XtFree(str);
          return;
        }
      }
      retcode = 0;
    }
    else if (chooseDir)
    {
      retcode = 0;
    }

    if (retcode != -1 && !iupCheck(h, IUP_NOCHANGEDIR))
    {
      getStrFromResource(w, XmNdirectory, dir);
      chdir(dir);
    }

    XtFree( str );

    {
      char v[5];
      sprintf(v, "%d", retcode);
      IupStoreAttribute(h, IUP_STATUS, v);
    }
  }

  /* caso o usuario tenha cancelado a operacao, temos que restaurar o valor
  do campo do diretorio
  */
  if (retcode == -1)
  {
    XmString xmstr;

    getcwd(dir, 256);
    xmstr = XmStringCreateLocalized(dir);
    XtVaSetValues( w, XmNdirectory, xmstr, NULL );
    XmStringFree( xmstr );
  }
  IupHide( h );
}

static void motRestoreFileName (Ihandle *h)
{
  char *filename = iupGetEnv(h, IUP_FILE);
  Widget w = (Widget)handle(child(h));
  if (filename != NULL)
  {
    if (XmVERSION < 2 && strrtok(filename, DIR_SEPARATOR) == NULL)
    {
      char dir[256];
      getStrFromResource(w, XmNdirectory, dir);
      XtVaSetValues(XmFileSelectionBoxGetChild(w, XmDIALOG_TEXT), XmNvalue,
        strcat(dir, filename), NULL);
    }
    else
      XtVaSetValues(XmFileSelectionBoxGetChild(w, XmDIALOG_TEXT),
      XmNvalue, filename, NULL);
  }
}

static void motFileDlgSet(Ihandle *h, char *attr, char *val)
{
  Widget w = NULL;
  char newval[128] = "";
  static int allow_recursion = 1;

  if(h == NULL || child(h) == NULL) return;
  w = (Widget)handle(child(h));

  if (!w) return;

  if (iupStrEqualNoCase(attr, IUP_DIRECTORY))
  {
    XmString xmstr;
    if (val)
      xmstr = XmStringCreateLocalized(val);
    else
    {
      /* teoricamente, bastaria setar XmNdirectory para NULL. mas, por
      algum motivo, o Motif do IRIX nao entende isso direito
      */
      char dir[256];
      getcwd(dir, 256);
      xmstr = XmStringCreateLocalized( dir );
    }
    XtVaSetValues( w, XmNdirectory, xmstr, NULL );
    XmStringFree( xmstr );
    motRestoreFileName(h);
    return;
  }

  if (iupStrEqualNoCase(attr, IUP_FILE))
  {
    XmString xmstr;
    char *dir = iupGetEnv( h, IUP_DIRECTORY );
    char *tmp;
    char newdir[256];
    if (!allow_recursion) return;
    if (!val) val = "";
    else if (val[0] != '/')
    {
      if (dir == NULL)
      {
        getStrFromResource(w, XmNdirectory, newdir);
        strcat(newdir, val);
      }
      else
        strcat(strcpy(newdir, dir), val);
    }
    else
    {
      strcpy(newdir, val);
    }

    tmp = strrtok(newdir, DIR_SEPARATOR);
    if (tmp != NULL)
    {
      strcpy(newval, tmp);
      tmp[0] = 0;  /* estamos alterando newdir */
    }

    xmstr = XmStringCreateLocalized(newval);
    XtVaSetValues( w, XmNdirSpec, xmstr, NULL );

    if (dir != NULL && iupStrEqualNoCase(newdir, dir))
    {
      allow_recursion = 0;
      IupStoreAttribute(h, IUP_FILE, newval);
      allow_recursion = 1;
    }
    XmStringFree( xmstr );

    return;
  }

  if (iupStrEqualNoCase(attr, IUP_FILTER))
  {
    if (val)
    {
      XmString xmstr;
      char *filter = val;
      char *p = strchr(val, ';');
      if (p) 
      {
        filter = (char*)malloc(sizeof(char*)*(p-val+1));
        if (!filter) return;
        strncpy( filter, val, p-val);
        filter[p-val]  = 0;
      }
      xmstr = XmStringCreateLocalized( filter );
      XtVaSetValues( w, XmNpattern, xmstr, NULL );
      XmStringFree( xmstr );
      if (filter != val) free( filter );
    }
    else
      XtVaSetValues( w, XmNpattern, NULL, NULL );
    motRestoreFileName(h);
    return;
  }

  if (iupStrEqualNoCase(attr, IUP_DIALOGTYPE))
  {
    char* language = IupGetLanguage();
    XmString xmstr;
    char *str = "Ok";
    iupSetEnv(h, "_IUPMOT_FILEDLG_DIRONLY_", IUP_NO);
    if (iupStrEqualNoCase(val, IUP_SAVE))
    {
      str = iupStrEqual(language, IUP_ENGLISH) ? "Save" : "Salva";
    }
    else if (iupStrEqualNoCase(val, IUP_OPEN))
    {
      str = iupStrEqual(language, IUP_ENGLISH) ? "Open" : "Abre";
    }
    else if (iupStrEqualNoCase(val, IUP_DIR))
    {
      str = iupStrEqual(language, IUP_ENGLISH) ? "Select" : "Seleciona";
      iupSetEnv(h, "_IUPMOT_FILEDLG_DIRONLY_", IUP_YES);
    }
    xmstr = XmStringCreateLocalized( str );
    XtVaSetValues( w, XmNokLabelString, xmstr, NULL );
    XmStringFree( xmstr );
    return;
  }

  iupdrvSetAttribute( h, attr, val );
}

static Ihandle* motFileDlgCreate(Iclass *ic)
{
  Ihandle* user = IupUser();
  Ihandle* ih   = IupDialog( user );

  IupSetAttribute( user, IUP_EXPAND, IUP_YES );
  IupSetAttribute( ih,   IUP_MINBOX, IUP_NO );
  IupSetAttribute( ih,   IUP_MAXBOX, IUP_NO );
  IupSetAttribute( ih,   IUP_RESIZE, IUP_NO );

  if ( iupStrEqual(IupGetLanguage(), IUP_ENGLISH))
    IupSetAttribute( ih, IUP_TITLE, "File Selection" );
  else
    IupSetAttribute( ih, IUP_TITLE, "Seleciona Arquivo" );
  return ih;
}

static void motFileDlgUpdateAttrib( Ihandle* h, char* attr )
{
  char *v = iupGetEnv(h, attr);
  if (v) motFileDlgSet( h, attr, v );
}

static int motFileDlgMap(Ihandle *h)
{
  Widget parent;
  Widget fileDialog;
  Pixel fg, bg;
  XmString dialogtitle = NULL, filterlabel = NULL, 
    dirlistlabel = NULL, filelistlabel = NULL, cancellabel = NULL,
    applylabel = NULL, selectionlabel = NULL;

  XmString basedir = NULL;

  char* language = IupGetLanguage();
  Arg arg[32];
  int args = 0;

  if (child(h) == NULL || handle(child(h)) == NULL)
  {
    iupCpiDefaultMap( h, 0 );

    parent = XtNameToWidget( (Widget)handle(h), "*dialog_area");

    XtVaSetValues( parent, XmNresizePolicy, XmRESIZE_ANY, NULL );

    XtVaGetValues( parent, XmNforeground, &fg, XmNbackground, &bg, NULL );

    XtSetArg( arg[args], XmNforeground, fg ); args++;
    XtSetArg( arg[args], XmNbackground, bg ); args++;

    XtSetArg( arg[args], XmNmustMatch,  False ); args++;
    XtSetArg( arg[args], XmNresizePolicy, XmRESIZE_NONE ); args++;
    XtSetArg( arg[args], XmNtextColumns, 50 ); args++;
#if (XmVERSION >= 2)
#ifndef NO_PATH_MODE_RELATIVE
    XtSetArg( arg[args], XmNpathMode, XmPATH_MODE_RELATIVE); args++;
#endif
#endif
#ifdef IRIX
    XtSetArg( arg[args], SgNuseEnhancedFSB, True ); args++;
#endif

    if (!iupStrEqual(language, IUP_ENGLISH))
    {
      dialogtitle    = XmStringCreateLocalized("Selecao de Arquivos");
      filterlabel    = XmStringCreateLocalized("Filtro");
      dirlistlabel   = XmStringCreateLocalized("Diretorios");
      filelistlabel  = XmStringCreateLocalized("Arquivos");
      cancellabel    = XmStringCreateLocalized("Cancela");
      applylabel     = XmStringCreateLocalized("Filtra");
      selectionlabel = XmStringCreateLocalized("Arquivo corrente");
      basedir        = XmStringCreateLocalized("Diretorio");

      XtSetArg( arg[args], XmNdialogTitle, dialogtitle); args++;
      XtSetArg( arg[args], XmNfilterLabelString, filterlabel); args++;
      XtSetArg( arg[args], XmNdirListLabelString, dirlistlabel); args++;
      XtSetArg( arg[args], XmNfileListLabelString, filelistlabel); args++;
      XtSetArg( arg[args], XmNcancelLabelString, cancellabel); args++;
      XtSetArg( arg[args], XmNapplyLabelString, applylabel); args++;
      XtSetArg( arg[args], XmNselectionLabelString, selectionlabel); args++;
#if (XmVERSION >= 2)
      XtSetArg( arg[args], XmNdirTextLabelString, basedir); args++;
#endif
    }

    fileDialog = XtCreateWidget("getfile",
      xmFileSelectionBoxWidgetClass, parent,
      arg, args );

    if (!iupStrEqual(language, IUP_ENGLISH))
    {
      XmStringFree( dialogtitle );
      XmStringFree( filterlabel );
      XmStringFree( dirlistlabel );
      XmStringFree( filelistlabel );
      XmStringFree( cancellabel );
      XmStringFree( applylabel );
      XmStringFree( selectionlabel );
#if (XmVERSION >= 2)
      XmStringFree( basedir );
#endif
    }

    if (!fileDialog) return IUP_ERROR;

    XtAddCallback( fileDialog, XmNcancelCallback,  motFileDlgFileCb, (XtPointer)h );
    XtAddCallback( fileDialog, XmNokCallback,      motFileDlgFileCb, (XtPointer)h );

    XtUnmanageChild(XmFileSelectionBoxGetChild(fileDialog,XmDIALOG_HELP_BUTTON));
    XtManageChild(fileDialog);

    handle(child(h)) = fileDialog;
  }
  /* os dois proximos atributos sao tratados especialmente porque queremos
  atribuir valores mesmo que o environment seja NULL
  */
  motFileDlgSet(h, IUP_DIRECTORY, iupGetEnv(h, IUP_DIRECTORY));
  motFileDlgSet(h, IUP_FILTER, iupGetEnv(h, IUP_FILTER));

  motFileDlgUpdateAttrib( h, IUP_DIALOGTYPE );
  motFileDlgUpdateAttrib( h, IUP_FILE );
  motFileDlgUpdateAttrib( h, IUP_FILTERINFO );
  motFileDlgUpdateAttrib( h, IUP_ALLOWNEW );
  motFileDlgUpdateAttrib( h, IUP_NOCHANGEDIR );
  motFileDlgUpdateAttrib( h, IUP_FILEEXIST );

  IupSetAttribute(h, IUP_VALUE, NULL );
  IupSetAttribute(h, IUP_STATUS, "-1");

  return IUP_NOERROR;
}

static int motFileDlgGetSize(Ihandle* n, int *w, int *h)
{  
  return 0;
}

/*
* Funcoes exportadas
*/

Ihandle *IupFileDlg(void)
{
  return IupCreate("filedlg");
}

void IupFileDlgOpen(void)
{
  Iclass *ICFileDlg = iupCpiCreateNewClass("filedlg", NULL);

  iupCpiSetClassMethod(ICFileDlg, ICPI_SETATTR, (Imethod) motFileDlgSet);
  iupCpiSetClassMethod(ICFileDlg, ICPI_MAP,     (Imethod) motFileDlgMap);
  iupCpiSetClassMethod(ICFileDlg, ICPI_CREATE,  (Imethod) motFileDlgCreate);
  iupCpiSetClassMethod(ICFileDlg, ICPI_GETSIZE, (Imethod) motFileDlgGetSize);
}

