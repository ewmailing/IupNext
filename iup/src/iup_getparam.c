/** \file
 * \brief IupGetParam
 *
 * See Copyright Notice in iup.h
 * $Id: iup_getparam.c,v 1.5 2008-12-04 18:06:31 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "iup.h"
#include "iupgetparam.h"
#include "iupspin.h"

#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_strmessage.h"


#define RAD2DEG  57.296   /* radians to degrees */


static int iParamButtonOK_CB(Ihandle* self)
{
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  IupSetAttribute(dialog, "STATUS", "1");
  if (cb && !cb(dialog, -1, IupGetAttribute(dialog, "USER_DATA")))
    return IUP_DEFAULT;
  else
    return IUP_CLOSE;
}

static int iParamButtonCancel_CB(Ihandle* self)
{
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  IupSetAttribute(dialog, "STATUS", "0");
  if (cb) cb(dialog, -3, IupGetAttribute(dialog, "USER_DATA"));
  return IUP_CLOSE;
}

static int iParamToggleAction_CB(Ihandle *self, int v)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  int old_v = iupAttribGetInt(param, "VALUE");

  if (v == 1)
    IupSetAttribute(param, "VALUE", "1");
  else
    IupSetAttribute(param, "VALUE", "0");

  if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
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

static int iParamTextAction_CB(Ihandle *self, int c, char *after)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  Ihandle* aux = (Ihandle*)IupGetAttribute(param, "AUXCONTROL");
  (void)c;
 
  iupAttribStoreStr(param, "VALUE", after);

  if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    /* Undo */
    iupAttribStoreStr(param, "VALUE", IupGetAttribute(self, "VALUE"));
    return IUP_IGNORE;
  }

  if (aux)
    IupStoreAttribute(aux, "VALUE", after);

  if (IupGetInt(self, "SPIN") && !IupGetInt(self, "SPINAUTO"))
  {
    float min = iupAttribGetFloat(param, "MIN");
    float step = iupAttribGetFloat(self, "_INCSTEP");
    float val;
    if (iupStrToFloat(after, &val))
      IupSetfAttribute(self, "SPINVALUE", "%d", (int)((val-min)/step + 0.5));
  }

  return IUP_DEFAULT;
}

static int iParamValAction_CB(Ihandle *self, double val)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* text = (Ihandle*)IupGetAttribute(self, "_TEXT");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  float old_value = iupAttribGetFloat(param, "VALUE");

  char* type = IupGetAttribute(param, "TYPE");
  if (iupStrEqual(type, "INTEGER"))
  {
    iupAttribSetStrf(param, "VALUE", "%d", (int)val);
  }
  else
  {
    if (iupAttribGetInt(param, "ANGLE"))
    {
      float old_angle;

      if (val == 0)
      {
        old_angle = iupAttribGetFloat(param, "VALUE");
        iupAttribSetStrf(param, "_OLD_ANGLE", "%g", old_angle);
      }
      else
        old_angle = iupAttribGetFloat(param, "_OLD_ANGLE");

      val = old_angle + val*RAD2DEG;

      if (iupAttribGetInt(param, "INTERVAL"))
      {
        float min = iupAttribGetFloat(param, "MIN");
        float max = iupAttribGetFloat(param, "MAX");
        if (val < min)
          val = min;
        if (val > max)
          val = max;
      }
      else if (iupAttribGetInt(param, "PARTIAL"))
      {
        float min = iupAttribGetFloat(param, "MIN");
        if (val < min)
          val = min;
      }
    }

    iupAttribSetStrf(param, "VALUE", "%g", val);
  }

  if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    /* Undo */
    iupAttribSetStrf(param, "VALUE", "%g", old_value);

    if (!iupAttribGetInt(param, "ANGLE"))
      IupSetfAttribute(self, "VALUE", "%g", old_value);

    /* there is no IUP_IGNORE for IupVal */
    return IUP_DEFAULT;
  }

  type = iupAttribGetStr(param, "TYPE");
  if (iupStrEqual(type, "INTEGER"))
    IupSetfAttribute(text, "VALUE", "%d", (int)val);
  else
    IupSetfAttribute(text, "VALUE", "%g", val);

  if (IupGetInt(text, "SPIN") && !IupGetInt(text, "SPINAUTO"))
  {
    float min = iupAttribGetFloat(param, "MIN");
    float step = iupAttribGetFloat(text, "_INCSTEP");
    float val = IupGetFloat(text, "VALUE");
    IupSetfAttribute(text, "SPINVALUE", "%d", (int)((val-min)/step + 0.5));
  }

  return IUP_DEFAULT;
}
static int iParamListAction_CB(Ihandle *self, char *t, int i, int v)
{
  (void)t;
  if (v == 1)
  {
    Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
    Ihandle* dialog = IupGetDialog(self);
    Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
    int old_i = iupAttribGetInt(param, "VALUE");

    iupAttribSetStrf(param, "VALUE", "%d", i-1);

    if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
    {
      /* Undo */
      iupAttribSetStrf(param, "VALUE", "%d", old_i);
      IupSetfAttribute(self, "VALUE", "%d", old_i+1);

      /* there is no IUP_IGNORE for IupList */
      return IUP_DEFAULT;
    }
  }

  return IUP_DEFAULT;
}
static int iParamFileButton_CB(Ihandle *self)
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
      iupAttribStoreStr(param, "VALUE", IupGetAttribute(fileDlg, "VALUE")) ;
   } 

   IupDestroy( fileDlg) ;

   return IUP_DEFAULT;
}

static int iParamSpinReal_CB(Ihandle *self, int pos)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");
  Ihandle* text = (Ihandle*)iupAttribGetStr(param, "CONTROL");
  float min = iupAttribGetFloat(param, "MIN");
  float max = iupAttribGetFloat(param, "MAX");
  float val, step = iupAttribGetFloat(text, "_INCSTEP");

  val = (float)pos*step + min;
  if (val < min)
    val = min;
  if (val > max)
    val = max;

  iupAttribSetStrf(param, "VALUE", "%g", (double)val);

  if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    return IUP_IGNORE;
  }

  IupSetfAttribute(text, "VALUE", "%g", (double)val);

  {
    Ihandle* aux = (Ihandle*)iupAttribGetStr(param, "AUXCONTROL");
    if (aux)
      IupSetfAttribute(aux, "VALUE", "%g", (double)val);
  }

  return IUP_DEFAULT;
}

static int iParamSpinInt_CB(Ihandle *self, int pos)
{
  Ihandle* param = (Ihandle*)IupGetAttribute(self, "_PARAM");
  Ihandle* dialog = IupGetDialog(self);
  Iparamcb cb = (Iparamcb)IupGetCallback(dialog, "PARAM_CB");

  iupAttribSetInt(param, "VALUE", pos);

  if (cb && !cb(dialog, iupAttribGetInt(param, "INDEX"), IupGetAttribute(dialog, "USER_DATA"))) 
  {
    return IUP_IGNORE;
  }

  {
    Ihandle* aux = (Ihandle*)iupAttribGetStr(param, "AUXCONTROL");
    if (aux)
      IupSetfAttribute(aux, "VALUE", "%g", (double)pos);
  }

  return IUP_DEFAULT;
}

static Ihandle* iParamCreateBox(Ihandle* param)
{
  Ihandle *box, *ctrl, *label;
  char *type;

  label = IupLabel(iupAttribGetStr(param, "TITLE"));

  type = iupAttribGetStr(param, "TYPE");
  if (iupStrEqual(type, "SEPARATOR"))
  {
    box = IupHbox(label, NULL);
    IupSetAttribute(box,"ALIGNMENT","ACENTER");
  }
  else
  {
    if (iupStrEqual(type, "STRING") && iupAttribGetInt(param, "MULTILINE"))
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

  type = iupAttribGetStr(param, "TYPE");
  if (iupStrEqual(type, "BOOLEAN"))
  {
    int value = iupAttribGetInt(param, "VALUE");
    if (value)
    {
      ctrl = IupToggle(iupAttribGetStr(param, "_TRUE"), "paramdlgToggle");
      IupSetAttribute(ctrl, "VALUE", "ON");
    }
    else
    {
      ctrl = IupToggle(iupAttribGetStr(param, "_FALSE"), "paramdlgToggle");
      IupSetAttribute(ctrl, "VALUE", "OFF");
    }
    IupSetCallback(ctrl, "ACTION", (Icallback)iParamToggleAction_CB);

    IupAppend(box, ctrl);
    iupAttribSetStr(param, "DATA_TYPE", "1");
  }
  else if (iupStrEqual(type, "SEPARATOR"))
  {
    ctrl = IupLabel("");
    IupSetAttribute(ctrl, "SEPARATOR", "HORIZONTAL");

    IupAppend(box, ctrl);
    iupAttribSetStr(param, "DATA_TYPE", "-1");
  }
  else if (iupStrEqual(type, "LIST"))
  {
    char str[20] = "1";
    int i = 1;
    ctrl = IupList(NULL);
    IupSetCallback(ctrl, "ACTION", (Icallback)iParamListAction_CB);
    IupSetAttribute(ctrl, "DROPDOWN", "YES");
    IupSetfAttribute(ctrl, "VALUE", "%d", iupAttribGetInt(param, "VALUE")+1);

    while (*iupAttribGetStr(param, str) != 0)
    {
      IupStoreAttribute(ctrl, str, iupAttribGetStr(param, str));
      i++;
      sprintf(str, "%d", i);
    }
    IupStoreAttribute(ctrl, str, NULL);

    IupAppend(box, ctrl);
    iupAttribSetStr(param, "DATA_TYPE", "1");
  }
  else if (iupStrEqual(type, "STRING"))
  {
    if (iupAttribGetInt(param, "MULTILINE"))
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
    IupSetCallback(ctrl, "ACTION", (Icallback)iParamTextAction_CB);
    IupStoreAttribute(ctrl, "VALUE", iupAttribGetStr(param, "VALUE"));

    {
      char* mask = iupAttribGetStr(param, "MASK");
      if (mask)
        IupStoreAttribute(ctrl, "MASK", mask);
    }

    iupAttribSetStr(param, "DATA_TYPE", "0");
    iupAttribSetStr(param, "EXPAND", "1");
  }
  else if (iupStrEqual(type, "FILE"))
  {
      Ihandle* aux;

      ctrl = IupText(NULL);
      IupSetAttribute(ctrl, "SIZE", "100x");
      IupSetAttribute(ctrl, "EXPAND", "HORIZONTAL");
      IupAppend(box, ctrl);

      IupSetCallback(ctrl, "ACTION", (Icallback)iParamTextAction_CB);
      IupStoreAttribute(ctrl, "VALUE", iupAttribGetStr(param, "VALUE"));

      iupAttribSetStr(param, "DATA_TYPE", "0");
      iupAttribSetStr(param, "EXPAND", "1");

      
      aux = IupButton("...", "");
      IupSetAttribute(aux, "EXPAND", "NO");

      IupSetCallback (aux, "ACTION", (Icallback)iParamFileButton_CB);
      IupSetAttribute(aux, "_PARAM", (char*)param);
      IupSetAttribute(aux, "_TEXT", (char*)ctrl);
      IupSetAttribute(aux, "EXPAND", "NO");

      IupAppend(box, aux); 
  
  }
  else /* INTEGER, REAL */
  {
    ctrl = IupText(NULL);
    IupSetCallback(ctrl, "ACTION", (Icallback)iParamTextAction_CB);
    IupStoreAttribute(ctrl, "VALUE", iupAttribGetStr(param, "VALUE"));

    type = iupAttribGetStr(param, "TYPE");
    if (iupStrEqual(type, "REAL"))
    {
      if (iupAttribGetInt(param, "INTERVAL"))
      {
        float min = iupAttribGetFloat(param, "MIN");
        float max = iupAttribGetFloat(param, "MAX");
        float step = iupAttribGetFloat(param, "STEP");
        float val = iupAttribGetFloat(param, "VALUE");
        if (step == 0) step = (max-min)/20.0f;
        IupSetfAttribute(ctrl, "MASKFLOAT", "%f:%f", (double)min, (double)max);

        IupSetAttribute(ctrl, "SPIN", "YES");
        IupSetAttribute(ctrl, "SPINAUTO", "NO");
        IupAppend(box, ctrl);
        IupSetCallback(ctrl, "SPIN_CB", (Icallback)iParamSpinReal_CB);
        IupSetAttribute(ctrl, "SPINMIN", "0");
        IupSetfAttribute(ctrl, "SPINMAX", "%d", (int)((max-min)/step + 0.5));
        IupSetfAttribute(ctrl, "SPINVALUE", "%d", (int)((val-min)/step + 0.5));

        iupAttribSetStrf(ctrl, "_INCSTEP", "%g", step);
      }
      else if (iupAttribGetInt(param, "PARTIAL"))
      {
        float min = iupAttribGetFloat(param, "MIN");
        if (min == 0)
          IupSetAttribute(ctrl, "MASK", IUP_MASK_UFLOAT);
        else
          IupSetfAttribute(ctrl, "MASKFLOAT", "%f:%f", (double)min, (double)1.0e10);
        IupAppend(box, ctrl);
      }
      else
      {
        IupSetAttribute(ctrl, "MASK", IUP_MASK_FLOAT);
        IupAppend(box, ctrl);
      }

      iupAttribSetStr(param, "DATA_TYPE", "2");
      IupSetAttribute(ctrl, "SIZE", "50x");
    }
    else /* INTEGER*/
    {
      IupSetAttribute(ctrl, "SPIN", "YES");
      IupSetAttribute(ctrl, "SPINAUTO", "YES");
      IupAppend(box, ctrl);
      IupSetCallback(ctrl, "SPIN_CB", (Icallback)iParamSpinInt_CB);

      if (iupAttribGetInt(param, "INTERVAL"))
      {
        int min = iupAttribGetInt(param, "MIN");
        int max = iupAttribGetInt(param, "MAX");
        int step = iupAttribGetInt(param, "STEP");
        if (step)
        {
          iupAttribSetStrf(ctrl, "_INCSTEP", "%d", step);
          IupSetfAttribute(ctrl, "SPININC", "%d", step);
        }
        IupSetfAttribute(ctrl, "SPINMAX", "%d", max);
        IupSetfAttribute(ctrl, "SPINMIN", "%d", min);
      }
      else if (iupAttribGetInt(param, "PARTIAL"))
      {
        int min = iupAttribGetInt(param, "MIN");
        if (min == 0)
          IupSetAttribute(ctrl, "MASK", IUP_MASK_UINT);
        else
          IupSetfAttribute(ctrl, "MASKINT", "%d:2147483647", min);
        IupSetfAttribute(ctrl, "SPINMIN", "%d", min);
        IupSetAttribute(ctrl, "SPINMAX", "2147483647");
      }
      else                             
      {
        IupSetAttribute(ctrl, "SPINMIN", "-2147483647");
        IupSetAttribute(ctrl, "SPINMAX", "2147483647");
        IupSetAttribute(ctrl, "MASK", IUP_MASK_INT);
      }

      iupAttribSetStr(param, "DATA_TYPE", "1");
      IupSetAttribute(ctrl, "SIZE", "50x");
    }

    if (iupAttribGetInt(param, "INTERVAL") || iupAttribGetInt(param, "ANGLE"))
    {
      Ihandle* aux;

      if (iupAttribGetInt(param, "ANGLE"))
      {
        aux = IupCreatep("dial", "HORIZONTAL", NULL);
        if (aux)
        {
          IupSetfAttribute(aux, "VALUE", "%g", (double)(iupAttribGetFloat(param, "VALUE")/RAD2DEG));
          IupSetAttribute(aux, "SIZE", "50x12");
        }
      }
      else
      {
        char* step;
        aux = IupVal("HORIZONTAL");
        IupStoreAttribute(aux, "MIN", IupGetAttribute(param, "MIN"));
        IupStoreAttribute(aux, "MAX", IupGetAttribute(param, "MAX"));
        IupStoreAttribute(aux, "VALUE", IupGetAttribute(param, "VALUE"));
        IupSetAttribute(aux, "EXPAND", "HORIZONTAL");
        iupAttribSetStr(param, "AUXCONTROL", (char*)aux);
        iupAttribSetStr(param, "EXPAND", "1");
        step = IupGetAttribute(param, "STEP");
        if (step)
          IupSetfAttribute(aux, "STEP", "%g", iupAttribGetFloat(param, "STEP")/(iupAttribGetFloat(param, "MAX")-iupAttribGetFloat(param, "MIN")));
        else if (iupStrEqual(type, "INTEGER"))
          IupSetfAttribute(aux, "STEP", "%g", 1.0/(iupAttribGetFloat(param, "MAX")-iupAttribGetFloat(param, "MIN")));
      }

      if (aux)
      {
        IupSetCallback(aux, "CHANGEVALUE_CB", (Icallback)iParamValAction_CB);
        IupSetAttribute(aux, "_PARAM", (char*)param);
        IupSetAttribute(aux, "_TEXT", (char*)ctrl);

        IupAppend(box, aux);
      }
    }
  }

  IupSetAttribute(box, "_PARAM", (char*)param);
  iupAttribSetStr(param, "CONTROL", (char*)ctrl);
  iupAttribSetStr(param, "LABEL", (char*)label);
  return box;
}

static Ihandle* IupParamDlgP(Ihandle** params)
{
  Ihandle *dialog, *button_ok, *button_cancel, 
          *dlg_box, *button_box, *param_box;
  int i, lbl_width, p, expand;

  button_ok = IupButton("OK", NULL);
  IupSetAttribute(button_ok, "PADDING", "20x0");
  IupSetCallback(button_ok, "ACTION", (Icallback)iParamButtonOK_CB);

  button_cancel = IupButton(iupStrMessageGet("IUP_CANCEL"), NULL);
  IupSetAttribute(button_cancel, "PADDING", "20x0");
  IupSetCallback(button_cancel, "ACTION", (Icallback)iParamButtonCancel_CB);
  
  param_box = IupVbox(NULL);

  i = 0; expand = 0;
  while (params[i] != NULL)
  {
    IupAppend(param_box, iParamCreateBox(params[i]));

    if (IupGetInt(params[i], "EXPAND"))
      expand = 1;

    i++;
  }

  button_box = IupHbox(
    IupFill(),
    button_ok,
    button_cancel,
    NULL);
  IupSetAttribute(button_box,"MARGIN","0x0");
  IupSetAttribute(button_box, "NORMALIZESIZE", "HORIZONTAL");

  dlg_box = IupVbox(
    IupFrame(param_box),
    button_box,
    NULL);
  IupSetAttribute(dlg_box, "MARGIN", "10x10");
  IupSetAttribute(dlg_box, "GAP", "5");

  dialog = IupDialog(dlg_box);

  IupSetAttribute(dialog, "MINBOX", "NO");
  IupSetAttribute(dialog, "MAXBOX", "NO");
  if (!expand) 
  {
    IupSetAttribute(dialog, "RESIZE", "NO");
    IupSetAttribute(dialog, "DIALOGFRAME", "YES");
    IupSetAttribute(dialog,"DIALOGHINT","YES");
  }
  IupSetAttributeHandle(dialog, "DEFAULTENTER", button_ok);
  IupSetAttributeHandle(dialog, "DEFAULTESC", button_cancel);
  IupSetAttribute(dialog, "TITLE", "ParamDlg");
  IupSetAttribute(dialog, "PARENTDIALOG", IupGetGlobal("PARENTDIALOG"));
  IupSetAttribute(dialog, "ICON", IupGetGlobal("ICON"));
  iupAttribSetStr(dialog, "OK", (char*)button_ok);
  iupAttribSetStr(dialog, "CANCEL", (char*)button_cancel);

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
      iupAttribSetStrf(params[i], "INDEX", "%d", p);
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
  iupAttribStoreStr(param, "TITLE", (char*)title);
  return param;
}

static void iParamGetBoolNames(char* interval, Ihandle* param)
{
  int i = 0, d = 0;
  char *falsestr, *truestr = NULL;

  if (*interval != '[')
  {
    iupAttribStoreStr(param, "_TRUE", iupStrMessageGet("IUP_TRUE"));
    iupAttribStoreStr(param, "_FALSE", iupStrMessageGet("IUP_FALSE"));
    return;
  }
  interval++;

  falsestr = interval;
  while (interval[i] != '\n' && interval[i] != '{')
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
    iupAttribStoreStr(param, "_TRUE", truestr);
    iupAttribStoreStr(param, "_FALSE", falsestr);
  }
  else
  {
    iupAttribStoreStr(param, "_TRUE", iupStrMessageGet("IUP_TRUE"));
    iupAttribStoreStr(param, "_FALSE", iupStrMessageGet("IUP_FALSE"));
  }
}
static void iParamGetFileOptions(char* options, Ihandle* param)
{
  int i = 0, d = 0;
  char *type = NULL, *filter = NULL, *directory = NULL, *nochangedir = NULL, *nooverwriteprompt = NULL;

  if (*options != '[')
  {
    iupAttribSetStr(param, "_DIALOGTYPE", type);
    iupAttribSetStr(param, "_FILTER", filter);
    iupAttribSetStr(param, "_DIRECTORY", directory);
    iupAttribSetStr(param, "_NOCHANGEDIR", nochangedir);
    iupAttribSetStr(param, "_NOOVERWRITEPROMPT", nooverwriteprompt);
    return;
  }
  options++;

  if (*options)  /* not empty */
    type = options;

  while (options[i] != '\n' && options[i] != '{')
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

  iupAttribStoreStr(param, "_DIALOGTYPE", type);
  iupAttribStoreStr(param, "_FILTER", filter);
  iupAttribStoreStr(param, "_DIRECTORY", directory);
  iupAttribStoreStr(param, "_NOCHANGEDIR", nochangedir);
  iupAttribStoreStr(param, "_NOOVERWRITEPROMPT", nooverwriteprompt);
}

static void iParamGetInterval(char* interval, Ihandle* param)
{
  int i = 0, d = 0;
  char *min, *max = NULL, *step = NULL;

  if (*interval != '[')
    return;
  interval++;

  min = interval;
  while (interval[i] != '\n' && interval[i] != '{')
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
    iupAttribSetStr(param, "PARTIAL", "1");
    iupAttribStoreStr(param, "MIN", min);
  }
  else if (d == 2 || d == 3)
  {
    iupAttribSetStr(param, "INTERVAL", "1");
    iupAttribStoreStr(param, "MIN", min);
    iupAttribStoreStr(param, "MAX", max);
    iupAttribStoreStr(param, "STEP", step);
  }
}

static int iParamGetListItems(char* list, Ihandle* param)
{
  int i = 0, d = 1;
  char str[20];

  if (*list != '|')
    return 0;
  list++;

  while (list[i] != '\n' && list[i] != '{')
  {
    if (list[i] == '|')
    {
      list[i] = 0;
      if (i == 0)
        break;

      sprintf(str, "%d", d);
      iupAttribStoreStr(param, str, list);

      d++;
      list += i+1;
      i = -1;
    }

    i++;
  }

  sprintf(str, "%d", d);
  iupAttribSetStr(param, str, "");
  return 1;
}

static void iParamGetStringMask(char* interval, Ihandle* param)
{
  int i = 0;
  while (interval[i] != '\n' && interval[i] != '{')
    i++;
  interval[i] = 0;

  iupAttribStoreStr(param, "MASK", interval);
}

static char *iParamGetStrLine(const char* format, int *line_size, int *title_end)
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
  char* line = iParamGetStrLine(format, line_size, &title_end);
  return &line[title_end+1];
}

static Ihandle *IupParamf(const char* format, int *line_size)
{
  Ihandle* param;
  char* line, *type_desc;
  int title_end = 0;

  line = iParamGetStrLine(format, line_size, &title_end);
  param = IupParam(line);
  type_desc = &line[title_end+1];

  switch(*type_desc)
  {
  case 'b':
    iupAttribSetStr(param, "TYPE", "BOOLEAN");
    iupAttribSetStr(param, "DATA_TYPE", "1");
    iParamGetBoolNames(type_desc+1, param);
    break;
  case 'l':
    iupAttribSetStr(param, "TYPE", "LIST");
    iupAttribSetStr(param, "DATA_TYPE", "1");
    if (!iParamGetListItems(type_desc+1, param))
      return NULL;
    break;
  case 'a':
    iupAttribSetStr(param, "TYPE", "REAL");
    iupAttribSetStr(param, "DATA_TYPE", "2");
    iupAttribSetStr(param, "ANGLE", "1");
    iParamGetInterval(type_desc+1, param);
    break;
  case 'm':
    iupAttribSetStr(param, "MULTILINE", "1");
  case 's':
    iupAttribSetStr(param, "TYPE", "STRING");
    iupAttribSetStr(param, "DATA_TYPE", "0");
    iParamGetStringMask(type_desc+1, param);
    break;
  case 'i':
    iupAttribSetStr(param, "TYPE", "INTEGER");
    iupAttribSetStr(param, "DATA_TYPE", "1");
    iParamGetInterval(type_desc+1, param);
    break;
  case 'r':
    iupAttribSetStr(param, "TYPE", "REAL");
    iupAttribSetStr(param, "DATA_TYPE", "2");
    iParamGetInterval(type_desc+1, param);
    break;
  case 'f':
    iupAttribSetStr(param, "TYPE", "FILE");
    iupAttribSetStr(param, "DATA_TYPE", "0");
    iParamGetFileOptions(type_desc+1, param);
    break;
  case 't':
    iupAttribSetStr(param, "TYPE", "SEPARATOR");
    iupAttribSetStr(param, "DATA_TYPE", "-1");
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

static void iParamDestroyAll(Ihandle **params)
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
      iupAttribSetStrf(params[i], "VALUE", "%d", *data_int);
      p++;
    }
    else if (data_type == 2)
    {
      float *data_float = (float*)(param_data[p]);
      if (!data_float) return 0;
      iupAttribSetStrf(params[i], "VALUE", "%g", *data_float);
      p++;
    }
    else if (data_type == 0)
    {
      char *data_str = (char*)(param_data[p]);
      if (!data_str) return 0;
      iupAttribStoreStr(params[i], "VALUE", data_str);
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
    iParamDestroyAll(params);
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

      data_type = iupAttribGetInt(param, "DATA_TYPE");
      if (data_type == 1)
      {
        int *data_int = (int*)(param_data[i]);
        *data_int = iupAttribGetInt(param, "VALUE");
        p++;
      }
      else if (data_type == 2)
      {
        float *data_float = (float*)(param_data[i]);
        *data_float = iupAttribGetFloat(param, "VALUE");
        p++;
      }
      else 
      {
        char *data_str = (char*)(param_data[i]);
        strcpy(data_str, IupGetAttribute(param, "VALUE"));
        p++;
      }
    }

    iParamDestroyAll(params);
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

