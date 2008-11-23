/** \file
 * \brief IupGetParam
 *
 * See Copyright Notice in iup.h
 * $Id: iup_getparam.c,v 1.2 2008-11-23 19:32:16 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "iup.h"
#include "iupgetparam.h"
#include "iupspin.h"

#include "iup_str.h"

#define RAD2DEG  57.296   /* radians to degrees */
#define STROK     (iupStrEqual(IupGetLanguage(), "ENGLISH") ? "OK"     : "Confirma")
#define STRCANCEL (iupStrEqual(IupGetLanguage(), "ENGLISH") ? "Cancel" : "Cancela")
#define STRTRUE   (iupStrEqual(IupGetLanguage(), "ENGLISH") ? "True"   : "Verdadeiro")
#define STRFALSE  (iupStrEqual(IupGetLanguage(), "ENGLISH") ? "False"  : "Falso")


static int cbButtonOK(Ihandle* self)
{
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  IupSetAttribute(dialog, "STATUS", "1");
  if (cb && !cb(dialog, -1, IupGetAttribute(dialog, "USER_DATA")))
    return IUP_DEFAULT;
  else
    return IUP_CLOSE;
}

static int cbButtonCancel(Ihandle* self)
{
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  IupSetAttribute(dialog, "STATUS", "0");
  if (cb) cb(dialog, -3, IupGetAttribute(dialog, "USER_DATA"));
  return IUP_CLOSE;
}

static int cbToggle(Ihandle *self, int v)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  int old_v = IupGetInt(param, "VALUE");

  if (v == 1)
    IupSetAttribute(param, "VALUE", "1");
  else
    IupSetAttribute(param, "VALUE", "0");

  if (cb && !cb(dialog, IupGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    /* Undo */
    if (old_v == 1)
    {
      IupSetAttribute(param, "VALUE", "1");
      IupSetAttribute(self, "VALUE", "1");
    }
    else
    {
      IupSetAttribute(param, "VALUE", "0");
      IupSetAttribute(self, "VALUE", "0");
    }

    /* there is no IUP_IGNORE for IupToggle */
    return IUP_DEFAULT;
  }

  /* update the interface */
  if (v == 1)
    IupStoreAttribute(self, "TITLE", IupGetAttribute(param, "_TRUE"));
  else
    IupStoreAttribute(self, "TITLE", IupGetAttribute(param, "_FALSE"));

  return IUP_DEFAULT;
}

static int cbText(Ihandle *self, int c, char *after)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  Ihandle* aux = (Ihandle*)IupGetAttribute(param, "AUXCONTROL");
  (void)c;
 
  IupStoreAttribute(param, "VALUE", after);

  if (cb && !cb(dialog, IupGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    /* Undo */
    IupStoreAttribute(param, "VALUE", IupGetAttribute(self, "VALUE"));
    return IUP_IGNORE;
  }

  if (aux)
    IupStoreAttribute(aux, "VALUE", after);

  return IUP_DEFAULT;
}

static int cbVal(Ihandle *self, double val)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* text = (Ihandle*)IupGetAttribute(self, "_TEXT");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  float old_value = IupGetFloat(param, "VALUE");

  char* type = IupGetAttribute(param, "TYPE");
  if (iupStrEqual(type, "INTEGER"))
  {
    IupSetfAttribute(param, "VALUE", "%d", (int)val);
  }
  else
  {
    if (IupGetInt(param, "ANGLE"))
    {
      float old_angle;

      if (val == 0)
      {
        old_angle = IupGetFloat(param, "VALUE");
        IupSetfAttribute(param, "_OLD_ANGLE", "%g", old_angle);
      }
      else
        old_angle = IupGetFloat(param, "_OLD_ANGLE");

      val = old_angle + val*RAD2DEG;

      if (IupGetInt(param, "INTERVAL"))
      {
        float min = IupGetFloat(param, "MIN");
        float max = IupGetFloat(param, "MAX");
        if (val < min)
          val = min;
        if (val > max)
          val = max;
      }
      else if (IupGetInt(param, "PARTIAL"))
      {
        float min = IupGetFloat(param, "MIN");
        if (val < min)
          val = min;
      }
    }

    IupSetfAttribute(param, "VALUE", "%g", val);
  }

  if (cb && !cb(dialog, IupGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    /* Undo */
    IupSetfAttribute(param, "VALUE", "%g", old_value);

    if (!IupGetInt(param, "ANGLE"))
      IupSetfAttribute(self, "VALUE", "%g", old_value);

    /* there is no IUP_IGNORE for IupVal */
    return IUP_DEFAULT;
  }

  type = IupGetAttribute(param, "TYPE");
  if (iupStrEqual(type, "INTEGER"))
    IupSetfAttribute(text, "VALUE", "%d", (int)val);
  else
    IupSetfAttribute(text, "VALUE", "%g", val);

  return IUP_DEFAULT;
}
static int cbList(Ihandle *self, char *t, int i, int v)
{
  (void)t;
  if (v == 1)
  {
    Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
    Ihandle* dialog = IupGetDialog(self);
    Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
    int old_i = IupGetInt(param, "VALUE");

    IupSetfAttribute(param, "VALUE", "%d", i-1);

    if (cb && !cb(dialog, IupGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
    {
      /* Undo */
      IupSetfAttribute(param, "VALUE", "%d", old_i);
      IupSetfAttribute(self, "VALUE", "%d", old_i+1);

      /* there is no IUP_IGNORE for IupList */
      return IUP_DEFAULT;
    }
  }

  return IUP_DEFAULT;
}
static int cbFileBt(Ihandle *self)
{
   Ihandle* param   = (Ihandle*)IupGetAttribute(self, "_PARAM");
   Ihandle* textbox = (Ihandle*)IupGetAttribute(self, "_TEXT");

   Ihandle* fileDlg = IupFileDlg() ;

   IupSetAttribute( fileDlg, "DIALOGTYPE", IupGetAttribute( param, "_DIALOGTYPE"));
   IupSetAttribute( fileDlg, "DIALOGTYPE", IupGetAttribute( param, "_DIALOGTYPE"));
   IupSetAttribute( fileDlg, "TITLE", IupGetAttribute( param, "TITLE"));
   IupSetAttribute( fileDlg, "FILTER", IupGetAttribute( param, "_FILTER"));
   IupSetAttribute( fileDlg, "DIRECTORY", IupGetAttribute( param, "_DIRECTORY"));
   IupSetAttribute( fileDlg, "NOCHANGEDIR", IupGetAttribute( param, "_NOCHANGEDIR"));
   IupSetAttribute( fileDlg, "NOOVERWRITEPROMPT", IupGetAttribute( param, "_NOOVERWRITEPROMPT"));
   IupSetAttribute( fileDlg, "VALUE", IupGetAttribute( param, "VALUE"));

   IupPopup(fileDlg, IUP_CENTER, IUP_CENTER) ;

   if( IupGetInt(fileDlg, "STATUS" ) != -1)
   {
      IupSetAttribute(textbox, "VALUE", IupGetAttribute(fileDlg, "VALUE")) ;
      IupStoreAttribute(param, "VALUE", IupGetAttribute(fileDlg, "VALUE")) ;
   } 

   IupDestroy( fileDlg) ;

   return IUP_DEFAULT;
}

static int cbSpin(Ihandle *self, int i)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  Ihandle* text = (Ihandle*)IupGetAttribute(param, "CONTROL");
  float val, step = IupGetFloat(text, "_INCSTEP");
  if (step == 0) step = 1;

  val = IupGetFloat(text, "VALUE") + i*step;

  if (IupGetInt(param, "INTERVAL"))
  {
    float min = IupGetFloat(param, "MIN");
    float max = IupGetFloat(param, "MAX");
    if (val < min)
      val = min;
    if (val > max)
      val = max;
  }
  else if (IupGetInt(param, "PARTIAL"))
  {
    float min = IupGetFloat(param, "MIN");
    if (val < min)
      val = min;
  }

  IupSetfAttribute(param, "VALUE", "%g", (double)val);

  if (cb && !cb(dialog, IupGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    return IUP_IGNORE;
  }

  IupSetfAttribute(text, "VALUE", "%g", (double)val);

  {
    Ihandle* aux = (Ihandle*)IupGetAttribute(param, "AUXCONTROL");
    if (aux)
      IupSetfAttribute(aux, "VALUE", "%g", (double)val);
  }

  return IUP_DEFAULT;
}

static Ihandle* param_create_box(Ihandle* param)
{
  Ihandle *box, *ctrl, *label;
  char *type;

  label = IupLabel(IupGetAttribute(param, "TITLE"));

  type = IupGetAttribute(param, "TYPE");
  if (iupStrEqual(type, "SEPARATOR"))
  {
    box = IupHbox(label, NULL);
    IupSetAttribute(box,"ALIGNMENT","ACENTER");
  }
  else
  {
    if (iupStrEqual(type, "STRING") && IupGetInt(param, "MULTILINE"))
    {
      Ihandle* hbox1 = IupHbox(IupSetAttributes(IupFill(), "SIZE=5"), label, NULL);
      IupSetAttribute(hbox1,"ALIGNMENT","ACENTER");

      box = IupVbox(hbox1, NULL);
      IupSetAttribute(box,"ALIGNMENT","ALEFT");
    }
    else
    {
      box = IupHbox(IupSetAttributes(IupFill(), "SIZE=5"), label, NULL);
      IupSetAttribute(box,"ALIGNMENT","ACENTER");
    }
  }

  IupSetAttribute(box,"MARGIN","0x0");

  type = IupGetAttribute(param, "TYPE");
  if (iupStrEqual(type, "BOOLEAN"))
  {
    int value = IupGetInt(param, "VALUE");
    if (value)
    {
      ctrl = IupToggle(IupGetAttribute(param, "_TRUE"), "paramdlgToggle");
      IupSetAttribute(ctrl, "VALUE", "ON");
    }
    else
    {
      ctrl = IupToggle(IupGetAttribute(param, "_FALSE"), "paramdlgToggle");
      IupSetAttribute(ctrl, "VALUE", "OFF");
    }
    IupSetCallback(ctrl, "ACTION", (Icallback)cbToggle);

    IupAppend(box, ctrl);
    IupSetAttribute(param, "DATA_TYPE", "1");
  }
  else if (iupStrEqual(type, "SEPARATOR"))
  {
    ctrl = IupLabel("");
    IupSetAttribute(ctrl, "SEPARATOR", "HORIZONTAL");

    IupAppend(box, ctrl);
    IupSetAttribute(param, "DATA_TYPE", "-1");
  }
  else if (iupStrEqual(type, "LIST"))
  {
    char str[20] = "1";
    int i = 1;
    ctrl = IupList(NULL);
    IupSetCallback(ctrl, "ACTION", (Icallback)cbList);
    IupSetAttribute(ctrl, "DROPDOWN", "YES");
    IupSetfAttribute(ctrl, "VALUE", "%d", IupGetInt(param, "VALUE")+1);

    while (*IupGetAttribute(param, str) != 0)
    {
      IupStoreAttribute(ctrl, str, IupGetAttribute(param, str));
      i++;
      sprintf(str, "%d", i);
    }
    IupStoreAttribute(ctrl, str, NULL);

    IupAppend(box, ctrl);
    IupSetAttribute(param, "DATA_TYPE", "1");
  }
  else if (iupStrEqual(type, "STRING"))
  {
    if (IupGetInt(param, "MULTILINE"))
    {
      Ihandle* hbox;

      ctrl = IupMultiLine(NULL);
      IupSetAttribute(ctrl, "SIZE", "100x50");
      IupSetAttribute(ctrl, "EXPAND", "YES");

      hbox = IupHbox(IupSetAttributes(IupFill(), "SIZE=5"), ctrl, NULL);
      IupSetAttribute(hbox,"ALIGNMENT","ACENTER");

      IupAppend(box, hbox);
    }
    else
    {
      ctrl = IupText(NULL);
      IupSetAttribute(ctrl, "SIZE", "100x");
      IupSetAttribute(ctrl, "EXPAND", "HORIZONTAL");
      IupAppend(box, ctrl);
    }
    IupSetCallback(ctrl, "ACTION", (Icallback)cbText);
    IupStoreAttribute(ctrl, "VALUE", IupGetAttribute(param, "VALUE"));

    {
      char* mask = IupGetAttribute(param, "MASK");
      if (mask)
        IupStoreAttribute(ctrl, "MASK", mask);
    }

    IupSetAttribute(param, "DATA_TYPE", "0");
    IupSetAttribute(param, "EXPAND", "1");
  }
  else if (iupStrEqual(type, "FILE"))
  {
      Ihandle* aux;

      ctrl = IupText(NULL);
      IupSetAttribute(ctrl, "SIZE", "100x");
      IupSetAttribute(ctrl, "EXPAND", "HORIZONTAL");
      IupAppend(box, ctrl);

      IupSetCallback(ctrl, "ACTION", (Icallback)cbText);
      IupStoreAttribute(ctrl, "VALUE", IupGetAttribute(param, "VALUE"));

      IupSetAttribute(param, "DATA_TYPE", "0");
      IupSetAttribute(param, "EXPAND", "1");

      
      aux = IupButton("...", "");
      IupSetAttribute(aux, "EXPAND", "NO");

      IupSetCallback (aux, "ACTION", (Icallback)cbFileBt);
      IupSetAttribute(aux, "_PARAM", (char*)param);
      IupSetAttribute(aux, "_TEXT", (char*)ctrl);
      IupSetAttribute(aux, "EXPAND", "NO");

      IupAppend(box, aux); 
  
  }
  else /* INTEGER, REAL */
  {
    ctrl = IupText(NULL);
    IupSetCallback(ctrl, "ACTION", (Icallback)cbText);
    IupStoreAttribute(ctrl, "VALUE", IupGetAttribute(param, "VALUE"));

    type = IupGetAttribute(param, "TYPE");
    if (iupStrEqual(type, "REAL"))
    {
      if (IupGetInt(param, "INTERVAL"))
      {
        Ihandle* spinbox = IupSpinbox(ctrl);
        float min = IupGetFloat(param, "MIN");
        float max = IupGetFloat(param, "MAX");
        float step = IupGetFloat(param, "STEP");
        IupSetfAttribute(ctrl, "MASKFLOAT", "%f:%f", (double)min, (double)max);
        IupAppend(box, spinbox);
        if (step)
          IupSetfAttribute(box, "_INCSTEP", "%g", step);
        else
          IupSetfAttribute(box, "_INCSTEP", "%g", (max-min)/20.);
        IupSetCallback(spinbox, "SPIN_CB", (Icallback)cbSpin);
      }
      else if (IupGetInt(param, "PARTIAL"))
      {
        float min = IupGetFloat(param, "MIN");
        if (min == 0)
          IupSetAttribute(ctrl, "MASK", IUPMASK_UFLOAT);
        else
          IupSetfAttribute(ctrl, "MASKFLOAT", "%f:%f", (double)min, (double)1.0e10);
        IupAppend(box, ctrl);
      }
      else
      {
        IupSetAttribute(ctrl, "MASK", IUPMASK_FLOAT);
        IupAppend(box, ctrl);
      }

      IupSetAttribute(param, "DATA_TYPE", "2");
      IupSetAttribute(ctrl, "SIZE", "50x");
    }
    else /* INTEGER*/
    {
      Ihandle* spinbox = IupSpinbox(ctrl);
      IupSetCallback(spinbox, "SPIN_CB", (Icallback)cbSpin);
      IupAppend(box, spinbox);
  
      if (IupGetInt(param, "INTERVAL"))
      {
        int min = IupGetInt(param, "MIN");
        int max = IupGetInt(param, "MAX");
        int step = IupGetInt(param, "STEP");
        if (step)
          IupSetfAttribute(box, "_INCSTEP", "%d", step);
        IupSetfAttribute(ctrl, "MASKINT", "%d:%d", min, max);
      }
      else if (IupGetInt(param, "PARTIAL"))
      {
        int min = IupGetInt(param, "MIN");
        if (min == 0)
          IupSetAttribute(ctrl, "MASK", IUPMASK_UINT);
        else
          IupSetfAttribute(ctrl, "MASKINT", "%d:%d", min, 2147483647);
      }
      else                             
        IupSetAttribute(ctrl, "MASK", IUPMASK_INT);

      IupSetAttribute(param, "DATA_TYPE", "1");
      IupSetAttribute(ctrl, "SIZE", "50x");
    }

    if (IupGetInt(param, "INTERVAL") || IupGetInt(param, "ANGLE"))
    {
      Ihandle* aux;

      if (IupGetInt(param, "ANGLE"))
      {
        aux = IupCreatep("dial", "HORIZONTAL", NULL);
        if (aux)
        {
          IupSetfAttribute(aux, "VALUE", "%g", (double)(IupGetFloat(param, "VALUE")/RAD2DEG));
          IupSetAttribute(aux, "SIZE", "50x12");
        }
      }
      else
      {
        char* step;
        aux = IupVal("HORIZONTAL");
        IupStoreAttribute(aux, "VALUE", IupGetAttribute(param, "VALUE"));
        IupStoreAttribute(aux, "MIN", IupGetAttribute(param, "MIN"));
        IupStoreAttribute(aux, "MAX", IupGetAttribute(param, "MAX"));
        IupSetAttribute(aux, "EXPAND", "HORIZONTAL");
        IupSetAttribute(param, "AUXCONTROL", (char*)aux);
        IupSetAttribute(param, "EXPAND", "1");
        step = IupGetAttribute(param, "STEP");
        if (step)
          IupSetfAttribute(aux, "STEP", "%g", IupGetFloat(param, "STEP")/(IupGetFloat(param, "MAX")-IupGetFloat(param, "MIN")));
        else if (iupStrEqual(type, "INTEGER"))
          IupSetfAttribute(aux, "STEP", "%g", 1.0/(IupGetFloat(param, "MAX")-IupGetFloat(param, "MIN")));
      }

      if (aux)
      {
        IupSetCallback(aux, "BUTTON_PRESS_CB", (Icallback)cbVal);
        IupSetCallback(aux, "MOUSEMOVE_CB", (Icallback)cbVal);
        IupSetAttribute(aux, "_PARAM", (char*)param);
        IupSetAttribute(aux, "_TEXT", (char*)ctrl);

        IupAppend(box, aux);
      }
    }
  }

  IupSetAttribute(box, "_PARAM", (char*)param);
  IupSetAttribute(param, "CONTROL", (char*)ctrl);
  IupSetAttribute(param, "LABEL", (char*)label);
  return box;
}

static Ihandle* IupParamDlgP(Ihandle** params)
{
  Ihandle *dialog, *button_ok, *button_cancel, 
          *dlg_box, *button_box, *param_box;
  int i, lbl_width, p, expand;

  button_ok = IupButton(STROK, NULL);
  IupSetAttribute(button_ok, "SIZE", "50x");
  IupSetCallback(button_ok, "ACTION", (Icallback)cbButtonOK);
  IupSetHandle("IupGetParamOkButton", button_ok );

  button_cancel = IupButton(STRCANCEL, NULL);
  IupSetAttribute(button_cancel, "SIZE", "50x");
  IupSetCallback(button_cancel, "ACTION", (Icallback)cbButtonCancel);
  IupSetHandle("IupGetParamCancelButton", button_cancel );

  param_box = IupVbox(NULL);

  i = 0; expand = 0;
  while (params[i] != NULL)
  {
    IupAppend(param_box, param_create_box(params[i]));

    if (IupGetInt(params[i], "EXPAND"))
      expand = 1;

    i++;
  }

  button_box = IupHbox(
    IupSetAttributes(IupFill(), "EXPAND=HORIZONTAL"),
    button_ok,
    button_cancel,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");

  dlg_box = IupVbox(
    IupFrame(param_box),
    IupSetAttributes(IupFill(), "SIZE=1x"),
    button_box,
    NULL);
  IupSetAttribute(dlg_box, "MARGIN", "10x10");
  IupSetAttribute(dlg_box, "GAP", "5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog, "MINBOX", "NO");
  IupSetAttribute(dialog, "MAXBOX", "NO");
  if (!expand) IupSetAttribute(dialog, "RESIZE", "NO");
  IupSetAttribute(dialog, "DEFAULTENTER", "IupGetParamOkButton");
  IupSetAttribute(dialog, "DEFAULTESC", "IupGetParamCancelButton");
  IupSetAttribute(dialog, "TITLE", "ParamDlg");
  IupSetAttribute(dialog, "PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dialog, "ICON", IupGetGlobal("ICON"));
  IupSetAttribute(dialog, "OK", (char*)button_ok);
  IupSetAttribute(dialog, "CANCEL", (char*)button_cancel);

  IupMap(dialog);

  i = 0; lbl_width = 0, p = 0;
  while (params[i] != NULL)
  {
    int w;

    char* type = IupGetAttribute(params[i], "TYPE");
    if (!iupStrEqual(type, "SEPARATOR"))
    {
      char str[20];
      sprintf(str, "PARAM%d", p);
      IupSetAttribute(dialog, str, (char*)params[i]);
      IupSetfAttribute(params[i], "INDEX", "%d", p);
      p++;
    }

    w = IupGetInt((Ihandle*)IupGetAttribute(params[i], "LABEL"), "SIZE");
    if (w > lbl_width)
      lbl_width = w;

    i++;
  }

  i = 0;
  while (params[i] != NULL)
  {
    char* type = IupGetAttribute(params[i], "TYPE");
    if (!iupStrEqual(type, "SEPARATOR"))
    {
      if (iupStrEqual(type, "LIST"))
      {
        Ihandle* ctrl = (Ihandle*)IupGetAttribute(params[i], "CONTROL");
        if (IupGetInt(ctrl, "SIZE") < 50)
          IupSetAttribute(ctrl, "SIZE", "50x");
      }
      else if (iupStrEqual(type, "BOOLEAN"))
      {
        Ihandle* ctrl = (Ihandle*)IupGetAttribute(params[i], "CONTROL");
        int lenF = strlen(IupGetAttribute(params[i], "_FALSE"));
        int lenT = strlen(IupGetAttribute(params[i], "_TRUE"));
        int w = IupGetInt(ctrl, "SIZE");
        int v = IupGetInt(ctrl, "VALUE");
        if (v)
        {
          int wf = (lenF*w)/lenT;
          if (wf > w)
            IupSetfAttribute(ctrl, "SIZE", "%dx", wf+8);
        }
        else
        {
          int wt = (lenT*w)/lenF;
          if (wt > w)
            IupSetfAttribute(ctrl, "SIZE", "%dx", wt+8);
        }
      }

      IupSetfAttribute((Ihandle*)IupGetAttribute(params[i], "LABEL"), "SIZE", "%dx", lbl_width);
    }

    i++;
  }

  IupSetAttribute(dialog, "SIZE", NULL);

  return dialog;
}

static Ihandle *IupParam(const char *title)
{
  Ihandle* param = IupUser();
  IupStoreAttribute(param, "TITLE", (char*)title);
  return param;
}

static void get_boolnames(char* interval, Ihandle* param)
{
  int i = 0, d = 0;
  char *falsestr, *truestr = NULL;

  if (*interval != '[')
  {
    IupStoreAttribute(param, "_TRUE", STRTRUE);
    IupStoreAttribute(param, "_FALSE", STRFALSE);
    return;
  }
  interval++;

  falsestr = interval;
  while (interval[i] != '\n')
  {
    if (interval[i] == ',' || interval[i] == ']')
    {
      interval[i] = 0;
      if (i == 0)
        break;

      d++;
      interval += i+1;

      if (d == 1)
        truestr = interval;

      i = -1;
      if (d == 2)
        break;
    }

    i++;
  }

  if (d == 2)
  {
    IupStoreAttribute(param, "_TRUE", truestr);
    IupStoreAttribute(param, "_FALSE", falsestr);
  }
  else
  {
    IupStoreAttribute(param, "_TRUE", STRTRUE);
    IupStoreAttribute(param, "_FALSE", STRFALSE);
  }
}
static void get_file_options(char* options, Ihandle* param)
{
  int i = 0, d = 0;
  char *type = NULL, *filter = NULL, *directory = NULL, *nochangedir = NULL, *nooverwriteprompt = NULL;

  if (*options != '[')
  {
    IupSetAttribute(param, "_DIALOGTYPE", type);
    IupSetAttribute(param, "_FILTER", filter);
    IupSetAttribute(param, "_DIRECTORY", directory);
    IupSetAttribute(param, "_NOCHANGEDIR", nochangedir);
    IupSetAttribute(param, "_NOOVERWRITEPROMPT", nooverwriteprompt);
    return;
  }
  options++;

  if (*options)  /* not empty */
    type = options;

  while (options[i] != '\n')
  {
    if (options[i] == '|' || options[i] == ']')
    {
      options[i] = 0;
      if (i == 0)
        break;

      d++;
      options+= i+1;

      if (d == 1 && *options)
        filter= options;
      else if (d == 2 && *options)
        directory= options;
      else if (d == 3 && *options)
        nochangedir= options;
      else if (d == 4 && *options)
        nooverwriteprompt = options;

      i = -1;
      if (d == 5)
        break;
    }

    i++;
  }

  IupStoreAttribute(param, "_DIALOGTYPE", type);
  IupStoreAttribute(param, "_FILTER", filter);
  IupStoreAttribute(param, "_DIRECTORY", directory);
  IupStoreAttribute(param, "_NOCHANGEDIR", nochangedir);
  IupStoreAttribute(param, "_NOOVERWRITEPROMPT", nooverwriteprompt);
}

static void get_interval(char* interval, Ihandle* param)
{
  int i = 0, d = 0;
  char *min, *max = NULL, *step = NULL;

  if (*interval != '[')
    return;
  interval++;

  min = interval;
  while (interval[i] != '\n')
  {
    if (interval[i] == ',' || interval[i] == ']')
    {
      interval[i] = 0;
      if (i == 0)
        break;

      d++;
      interval += i+1;

      if (d == 1)
        max = interval;

      if (d == 2)
        step = interval;

      i = -1;
      if (d == 3)
        break;
    }

    i++;
  }

  if (d == 1)
  {
    IupSetAttribute(param, "PARTIAL", "1");
    IupStoreAttribute(param, "MIN", min);
  }
  else if (d == 2 || d == 3)
  {
    IupSetAttribute(param, "INTERVAL", "1");
    IupStoreAttribute(param, "MIN", min);
    IupStoreAttribute(param, "MAX", max);
    IupStoreAttribute(param, "STEP", step);
  }
}

static int get_list_items(char* list, Ihandle* param)
{
  int i = 0, d = 1;
  char str[20];

  if (*list != '|')
    return 0;
  list++;

  while (list[i] != '\n')
  {
    if (list[i] == '|')
    {
      list[i] = 0;
      if (i == 0)
        break;

      sprintf(str, "%d", d);
      IupStoreAttribute(param, str, list);

      d++;
      list += i+1;
      i = -1;
    }

    i++;
  }

  sprintf(str, "%d", d);
  IupSetAttribute(param, str, "");
  return 1;
}

static void get_string_mask(char* interval, Ihandle* param)
{
  int i = 0;
  while (interval[i] != '\n')
    i++;
  interval[i] = 0;

  IupStoreAttribute(param, "MASK", interval);
}

static char *iupParamLine(const char* format, int *line_size, int *title_end)
{
  static char line[4096];
  int i = 0;

  *title_end = 0;

  while (format[i] != '\n')
  {
    line[i] = format[i];
    if (line[i] == '%')
      *title_end = i;
    i++;
    if (i > 4000)
      break;
  }
  line[i] = '\n';
  line[i+1] = 0;
  *line_size = i+1;

  if (*title_end == 0)
    return NULL;

  line[*title_end] = 0;

  return line; 
}

char *IupGetParamType(const char* format, int *line_size)
{
  int title_end = 0;
  char* line = iupParamLine(format, line_size, &title_end);
  return &line[title_end+1];
}

static Ihandle *IupParamf(const char* format, int *line_size)
{
  Ihandle* param;
  char* line, *type_desc;
  int title_end = 0;

  line = iupParamLine(format, line_size, &title_end);
  param = IupParam(line);
  type_desc = &line[title_end+1];

  switch(*type_desc)
  {
  case 'b':
    IupSetAttribute(param, "TYPE", "BOOLEAN");
    IupSetAttribute(param, "DATA_TYPE", "1");
    get_boolnames(type_desc+1, param);
    break;
  case 'l':
    IupSetAttribute(param, "TYPE", "LIST");
    IupSetAttribute(param, "DATA_TYPE", "1");
    if (!get_list_items(type_desc+1, param))
      return NULL;
    break;
  case 'a':
    IupSetAttribute(param, "TYPE", "REAL");
    IupSetAttribute(param, "DATA_TYPE", "2");
    IupSetAttribute(param, "ANGLE", "1");
    get_interval(type_desc+1, param);
    break;
  case 'm':
    IupSetAttribute(param, "MULTILINE", "1");
  case 's':
    IupSetAttribute(param, "TYPE", "STRING");
    IupSetAttribute(param, "DATA_TYPE", "0");
    get_string_mask(type_desc+1, param);
    break;
  case 'i':
    IupSetAttribute(param, "TYPE", "INTEGER");
    IupSetAttribute(param, "DATA_TYPE", "1");
    get_interval(type_desc+1, param);
    break;
  case 'r':
    IupSetAttribute(param, "TYPE", "REAL");
    IupSetAttribute(param, "DATA_TYPE", "2");
    get_interval(type_desc+1, param);
    break;
  case 'f':
    IupSetAttribute(param, "TYPE", "FILE");
    IupSetAttribute(param, "DATA_TYPE", "0");
    get_file_options(type_desc+1, param);
    break;
  case 't':
    IupSetAttribute(param, "TYPE", "SEPARATOR");
    IupSetAttribute(param, "DATA_TYPE", "-1");
    break;

  default:
    return NULL;
  }

  return param;
}

int IupGetParamCount(const char *format, int *param_extra)
{
  int param_count = 0, sep = 0;
  const char* s = format;

  *param_extra = 0;
  while(*s)
  {
    if (*s == '%' && *(s+1) == 't')  /* do not count separator lines */
    {
      sep = 1;
      (*param_extra)++;
    }

    if (*s == '\n')
    {
      if (sep)
        sep = 0;
      else
        param_count++;
    }

    s++;
  }

  return param_count;
}

static void DestroyParams(Ihandle **params)
{
  int i = 0;
  while (params[i] != NULL)
  {
    IupDestroy(params[i]);
    i++;
  }
}

int IupGetParamv(const char* title, Iparamcb action, void* user_data, const char* format, int param_count, int param_extra, void** param_data)
{
  Ihandle *param_dlg, *params[50];
  int i, line_size, p;

  assert(title && format);
  if (!title || !format)
    return 0;

  for (i = 0, p = 0; i < param_count+param_extra; i++)
  {
    int data_type;

    params[i] = IupParamf(format, &line_size);
    assert(params[i]);
    if (!params[i])
      return 0;

    data_type = IupGetInt(params[i], "DATA_TYPE");
    if (data_type == 1)
    {
      int *data_int = (int*)(param_data[p]);
      if (!data_int) return 0;
      IupSetfAttribute(params[i], "VALUE", "%d", *data_int);
      p++;
    }
    else if (data_type == 2)
    {
      float *data_float = (float*)(param_data[p]);
      if (!data_float) return 0;
      IupSetfAttribute(params[i], "VALUE", "%g", *data_float);
      p++;
    }
    else if (data_type == 0)
    {
      char *data_str = (char*)(param_data[p]);
      if (!data_str) return 0;
      IupStoreAttribute(params[i], "VALUE", data_str);
      p++;
    }

    format += line_size; 
  }
  params[i] = NULL;

  param_dlg = IupParamDlgP(params);
  IupSetAttribute(param_dlg, "TITLE", (char*)title);
  IupSetCallback(param_dlg, "PARAM_CB", (Icallback)action);
  IupSetAttribute(param_dlg, "USER_DATA", (char*)user_data);

  if (action) 
    action(param_dlg, -2, user_data);

  IupPopup(param_dlg, IUP_CENTER, IUP_CENTER);

  if (!IupGetInt(param_dlg, "STATUS"))
  {
    DestroyParams(params);
    IupDestroy(param_dlg);
    return 0;
  }
  else
  {
    for (i = 0, p = 0; i < param_count; i++)
    {
      Ihandle* param;
      int data_type;
      char str[20];

      sprintf(str, "PARAM%d", i);
      param = (Ihandle*)IupGetAttribute(param_dlg, str);

      data_type = IupGetInt(param, "DATA_TYPE");
      if (data_type == 1)
      {
        int *data_int = (int*)(param_data[i]);
        *data_int = IupGetInt(param, "VALUE");
        p++;
      }
      else if (data_type == 2)
      {
        float *data_float = (float*)(param_data[i]);
        *data_float = IupGetFloat(param, "VALUE");
        p++;
      }
      else 
      {
        char *data_str = (char*)(param_data[i]);
        strcpy(data_str, IupGetAttribute(param, "VALUE"));
        p++;
      }
    }

    DestroyParams(params);
    IupDestroy(param_dlg);
    return 1;
  }
}

int IupGetParam(const char* title, Iparamcb action, void* user_data, const char* format,...)
{
  int param_count, param_extra, i;
  void* param_data[50];
  va_list arg;

  param_count = IupGetParamCount(format, &param_extra);

  va_start(arg, format);
  for (i = 0; i < param_count; i++)
  {
    param_data[i] = (void*)(va_arg(arg, void*));
  }
  va_end(arg);

  return IupGetParamv(title, action, user_data, format, param_count, param_extra, param_data);
}

