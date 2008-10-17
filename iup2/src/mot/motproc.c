/** \file
 * \brief Motif Driver Callback management
 *
 * See Copyright Notice in iup.h
 * $Id: motproc.c,v 1.1 2008-10-17 06:19:25 scuri Exp $
 */

#if defined(__STDC__) && defined(ULTRIX)
   #define X$GBLS(x)  #x
#endif

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>

#if (XmVERSION < 2)
#include "ComboBox1.h"
#else
#include <Xm/ComboBox.h>
#endif

#include "iupkey.h"
#include "iglobal.h"
#include "imask.h"
#include "idrv.h"
#include "motif.h"
#include "mottips.h"

#define RESIZE_FIRST_TIME "RSZFRSTTM"

int iupmot_incallback = TRUE;   
static Ihandle* iupmot_post_killfocus = FALSE;   

static void xrepaint(Widget w, XExposeEvent *ev);
static void xinput(Widget w, XEvent *event);

void iupmotCBcreatecanvas(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
  Ihandle *n = (Ihandle*)data;

  if (ev->type != CreateNotify) return;

  if (ev->xcreatewindow.window == XtWindow((Widget) handle(n)))
  {
    Icallback cb = IupGetCallback(n, IUP_MAP_CB);
    XSetWindowAttributes attr;

    /* Seta o bit de gravidade para ForgetGravity e
     * habilita backing store */

    attr.bit_gravity = ForgetGravity;
    if (iupCheck(n, "BACKINGSTORE"))
      attr.backing_store = WhenMapped;
    else
      attr.backing_store = NotUseful;
    attr.background_pixel = None;
    XChangeWindowAttributes( iupmot_display, ev->xcreatewindow.window,
        CWBackingStore | CWBitGravity, &attr );

    iupdrvSetAttribute(n, IUP_CURSOR, IupGetAttribute(n, IUP_CURSOR));

    if (cb)
    {
      iupmot_incallback = 1;
      if (cb (n)==IUP_CLOSE) iupmot_exitmainloop = 1;
      iupmot_incallback = 0;
    }
  }
}

static void dlgResize(Widget w, Ihandle *n)
{
   Dimension width, height;
 
   XtVaGetValues (w, 
     XmNwidth, &width,
     XmNheight, &height,
    NULL);

   XtRemoveCallback (w, XmNresizeCallback, iupmotCBdialog, NULL);
   {  
      static char v[10];
      sprintf (v,"%dx%d", width, height);
      iupStoreEnv(n,IUP_RASTERSIZE,v);
      iupSetEnv(n,IUP_SIZE,NULL);
      if (iupSetSize (n) != IUP_ERROR)
      {
         iupdrvResizeObjects (child(n));
      }
   } 
   XtAddCallback (w, XmNresizeCallback, iupmotCBdialog, NULL);
}

void iupmotCBstructure (Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
  Ihandle *n = (Ihandle*)data;
  Widget form = XtNameToWidget((Widget)handle(n),"*dialog_area");
  Iwidgetdata *wd;
  int state = -1;
  if (!form) return;
  XtVaGetValues (form, XmNuserData, &wd, NULL);
  if (!wd) return;

  switch(ev->type)
  {
    case MapNotify:
    {
      if ( iconic( wd->data ) ) /* IUP_RESTORE */
      {
        wd->data = (void*)(-((long)wd->data));
        state = IUP_RESTORE;
      }
      else /* IUP_SHOW */
      {
        Ihandle *startfocus = IupGetAttributeHandle(n, "STARTFOCUS");
        if (startfocus)
          IupSetFocus(startfocus);

        state = IUP_SHOW;
        iupmot_nvisiblewin++;
      }
      break;
    }

    case UnmapNotify:
    {
      if ( shown(wd->data) || popuped(wd->data) ) /* IUP_MINIMIZE */
      {
        wd->data = (void*)(-((long)wd->data));
        state = IUP_MINIMIZE;
      }
      break;
    }
  }
  {
    IFni cb = (IFni)IupGetCallback(n, IUP_SHOW_CB);
    if (cb && state>=0)
    {
      iupmot_incallback = TRUE;
      if (cb(n, state) == IUP_CLOSE) 
        iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
    }
  }
}

void iupmotCBlist (Widget w, XtPointer client_data, XtPointer call_data)
{
   IFns ms_cb;
   IFnsii cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;
 
   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;

   cb = (IFnsii) IupGetCallback(n,IUP_ACTION);
   ms_cb = (IFns)IupGetCallback(n,"MULTISELECT_CB");
   if (cb || ms_cb)
   {
      char opstr[10];
      char *old_pos;
      int option;
      int old_opt;
      char *str, *old_str;

      if (XtClass(w) == xmComboBoxWidgetClass)
      {
         option = ((XmComboBoxCallbackStruct*)call_data)->item_position;
      }
      else /* XmList */
      {
         XmListCallbackStruct *data = (XmListCallbackStruct *)call_data;
         option = data->item_position;

         if (data->reason != XmCR_BROWSE_SELECT)
         {
            /* Multiple */
            int pos = 0;
            char *old_value = iupGetEnv(n, "_IUPMOT_LISTOLDVALUE");
            if (ms_cb)
            {
              char *ms_value = iupStrDup(IupGetAttribute(n, IUP_VALUE));
              if (old_value)
              {
                while(ms_value[pos] != '\0')
                {
                  if(ms_value[pos] == old_value[pos])
                    ms_value[pos] = 'x';    /* mark unchanged values */
                  pos++;
                }
              }

              iupmot_incallback = TRUE;
              if (ms_cb(n, ms_value) == IUP_CLOSE)
                iupmot_exitmainloop = 1;
              iupmot_incallback = FALSE;

              iupStoreEnv(n, "_IUPMOT_LISTOLDVALUE", IupGetAttribute(n, IUP_VALUE));
              free(ms_value);
              return;
            }
            else
            {
              char *new_value = iupStrDup(IupGetAttribute(n, IUP_VALUE));

              /* call list callback for each changed item */
              while(new_value[pos] != '\0')
              {
                int select = 0;

                if(!old_value && new_value[pos] == '+')
                  select = 1;
                else if(old_value && new_value[pos] == '+' &&
                                     old_value[pos] == '-')
                  select = 1;
                else if(old_value && new_value[pos] == '-' &&
                                     old_value[pos] == '+')
                  select = 0;
                else
                {
                  pos++;
                  continue;
                }

                sprintf(opstr, "%d", pos+1);
                str = iupGetEnv(n, opstr);

                iupmot_incallback = TRUE;
                if (cb(n, str, pos+1, select) == IUP_CLOSE)
                {
                  iupmot_exitmainloop = 1;
                  break;
                }
                iupmot_incallback = FALSE;

                pos++;
              }

              iupStoreEnv(n, "_IUPMOT_LISTOLDVALUE", new_value);
              free(new_value);
              return;
            }
         }
      }

      if (!cb) return;
     
      if (!iupStrEqualNoCase(IupGetAttribute(n, IUP_MULTIPLE), IUP_YES))
      {
        old_pos = iupGetEnv(n, "_IUPMOT_PREV_LIST_POS");
        if (!old_pos) old_pos = "1";
        sscanf(old_pos, "%d", &old_opt);
        old_str = iupGetEnv(n, old_pos);
        iupmot_incallback = TRUE;
        if (cb(n, old_str, old_opt, 0) == IUP_CLOSE)
           iupmot_exitmainloop = 1;
        iupmot_incallback = FALSE;
        
        sprintf(opstr, "%d", option);
        iupStoreEnv(n, "_IUPMOT_PREV_LIST_POS", opstr);
      }

      sprintf(opstr, "%d", option);
      str = iupGetEnv(n, opstr);

      iupmot_incallback = TRUE;
      if (cb(n, str, option, 1) == IUP_CLOSE)
        iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

static void callMaskCallback(Ihandle* h, int tipo)
{
   IFni cb;

   cb=(IFni)IupGetCallback(h,IUP_MASK_CB);

   if (cb)
   {
      iupmot_incallback = TRUE;
      cb(h,tipo);
      iupmot_incallback = FALSE;
   }
   /* Se tipo = 0, um caracter invalido foi digitado */
   /* Se tipo = 1, o campo perdeu o foco, mas a mascara esta incompleta */
}

static char* newString(XmTextVerifyPtr t, char* old)
{
   static char result[300];
   strcpy(result, old);
   if (t->text->ptr)
   {
      int i=0;
      while (i<t->text->length) 
      {
        *(result+t->startPos+i) = t->text->ptr[i];
        i++;
      }
      strcpy(result+t->startPos+i, old+t->endPos);
   }
   else
   {
      strcpy(result+t->startPos, old+t->endPos);
   }
   return result;
}

static char fill[40] = { 0 };

void iupmotCBmask(Widget w, XtPointer client_data, XtPointer call_data)
{
   if (fill[0])
   {
      XmTextFieldInsert(w, XmTextFieldGetLastPosition(w), fill);
      fill[0] = 0;
   }
}

static void predictTextState (Widget w, XmTextVerifyPtr newState,
                              int c, char *oldText, char *newText, int pos)
{
  static int inspos;
  int oldpos;

  if (pos == 0)
  {
    inspos = newState->startPos;
    oldpos = newState->endPos;
  }
  else
  {
    oldpos = inspos;
  }
  if (inspos > 0) strncpy(newText, oldText, inspos);
  if (c != K_BS && c != K_DEL) newText[inspos++] = c;
  newText[inspos] = 0;
  strcat(newText, &oldText[oldpos]);
}

void iupmotCBtext (Widget w, XtPointer client_data, XtPointer call_data)
{
  IFnis cb;
  Iwidgetdata *d = NULL;
  Ihandle *n;
  XmTextVerifyPtr text = (XmTextVerifyPtr)call_data;
  char *tmpStr, *oldText = NULL, *newText = NULL;

  if (iupmot_incallback) return;

  XtVaGetValues (w, XmNuserData, &d, NULL);
  if (!d) return;
  n = d->ihandle;

  if (n == NULL) return;

  if (type(n) != TEXT_ && type(n) != MULTILINE_ && type(n) != LIST_) return;

  if (type(n) == LIST_)
    cb = (IFnis) IupGetCallback(n, "EDIT_CB");
  else
    cb = (IFnis) IupGetCallback(n, IUP_ACTION);

  if (cb)
  {
    int result;
    int length = text->reason != XmCR_ACTIVATE ? text->text->length : 0;
    tmpStr = (type(n) == MULTILINE_)? XmTextGetString(w): XmTextFieldGetString(w);
    oldText = (char *)malloc(strlen(tmpStr)+length+1);
    newText = (char *)malloc(strlen(tmpStr)+length+1);
    if (oldText == NULL || newText == NULL)
    {
      fprintf(stderr, "IUP: Not enough memory, system may be unstable\n");
      return;
    }
    strcpy(oldText, tmpStr);
    newText[0] = 0;
    XtFree(tmpStr);

    if (text->reason == XmCR_ACTIVATE)
    {
      iupmot_incallback = TRUE;
      result = cb(n, K_CR, oldText);
      iupmot_incallback = FALSE;
      if (result == IUP_CLOSE)
        iupmot_exitmainloop = 1;
      else if (result == 9)
        XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
    }
    else
    {
      int i=0, del=0;
      iupmot_incallback = TRUE;
      if (text->text->length == 0)
      {
        if (text->event) 
          /* apesar de a documentacao dizer que este campo 
          sempre contera' um evento, no SunOS ele pode vir zero
          `as vezes... por isso o teste, senao voa */
        {
          int val = iupmotKeyGetCode(&(text->event->xkey));
          predictTextState(w, text, val, oldText, newText, 0);
          switch (cb(n, val, newText))
          {
          case IUP_CLOSE:
            text->doit =0;
            iupmot_exitmainloop = 1;
            break;
          case IUP_IGNORE:
            text->doit =0;
            break;
          case IUP_DEFAULT:
            break;
          case 9:
            text->doit =0;
            XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
            break;
          default:
            break;
          }
        }
      }

      for (i=0; i<text->text->length; i++)
      {
        int val = text->text->ptr[i] == '\n' ? K_CR : text->text->ptr[i];
        predictTextState(w, text, val, oldText, newText, i);
        result = cb(n, val, newText);
        switch (result)
        {
        case IUP_CLOSE:
          if (!i) text->doit =0;
          iupmot_exitmainloop = 1;
          break;
        case IUP_IGNORE:
          if (text->text->length==1) text->doit =0;
          else
          {
            int j;
            del++;
            for (j=i; j<text->text->length-del; j++)
              text->text->ptr[j] = text->text->ptr[j+1];
          }
          break;
        case IUP_DEFAULT:
          strcpy(oldText, newText);
          break;
        case 9:
          if (!i) text->doit =0;
          XmProcessTraversal(w, XmTRAVERSE_NEXT_TAB_GROUP);
          break;
        default:
          if (text->text->ptr[i] != result)
          {
            text->text->ptr[i] = result;
            predictTextState(w, text, result, oldText, newText, i);
            strcpy(oldText, newText);
          }
          break;
        }
      }
      text->text->length -= del;
      iupmot_incallback = FALSE;
      free(oldText);
      free(newText);
    }
  }

  if (d->data && text->doit) /* tem mascara */
  {
    char *str;
    ITextInfo *info = (ITextInfo*)d->data;

    str = (type(n) == MULTILINE_)? newString(text, XmTextGetString(w)) : newString(text, XmTextFieldGetString(w));
    info->status = iupCheckMask(info->mask, str, fill);
    if (info->status == 0) /* nao aceitou */
    {
      text->doit = 0;
      callMaskCallback(n, 0);
    }
    else if (!text->text->ptr && fill[0])
    {
      fill[0] = 0;
      info->status = 2;
    }
  }
}

static void makeStateString( unsigned int state, unsigned int bot, char *r )
{
   if (state & ShiftMask)   *r++ = 'S';
   else                     *r++ = ' ';
   if (state & ControlMask) *r++ = 'C';
   else                     *r++ = ' ';
   if ((state & Button1Mask) || bot==Button1) {*r++=IUP_BUTTON1;}
   else                     *r++ = ' ';
   if ((state & Button2Mask) || bot==Button2) {*r++=IUP_BUTTON2;}
   else                     *r++ = ' ';
   if ((state & Button3Mask) || bot==Button3) {*r++=IUP_BUTTON3;}
   else                     *r++ = ' ';
   *r='\0';
}

void iupmotCBbuttoncb (Widget w, XtPointer data, XEvent* event, Boolean* cont)
{
   unsigned long elapsed;
   static Time last = 0;
   static char report[12];

   Ihandle* n = (Ihandle*)data;
   IFniiiis cb = (IFniiiis) IupGetCallback(n, IUP_BUTTON_CB);
   XButtonEvent* ev = (XButtonEvent*)event;
   if ( ev->button!=Button1 &&
        ev->button!=Button2 &&
        ev->button!=Button3 ) 
     return;
   
   if (ev->type == ButtonPress)
     XmProcessTraversal(w, XmTRAVERSE_CURRENT);

   if (cb)
   {
      int b = (ev->button==Button1) ? '1' :
       ((ev->button==Button2) ? '2' : '3' );
      makeStateString(ev->state, ev->button, report);

      /* Double/Single Click */
      if (ev->type == ButtonPress)
      {
        elapsed = ev->time - last;
        last = ev->time;
        if (elapsed <= XtGetMultiClickTime(iupmot_display))
          strcat(report,"D");   /* Double_click */
        else
          strcat(report,"S");   /* Single_click */
      }

      iupmot_incallback = TRUE;

      if ( cb (n, b, (ev->type==ButtonPress), ev->x_root, ev->y_root, report)==IUP_CLOSE)
        iupmot_exitmainloop = 1;

      iupmot_incallback = FALSE;
   }
}

void iupmotCBhelp (Widget w, XtPointer client_data, XtPointer call_data)
{
   IFn cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;

   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;
   cb = (IFn) IupGetCallback(n, IUP_HELP_CB);
   if (cb)
   {
      iupmot_incallback = TRUE;
      if (cb(n) == IUP_CLOSE) 
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

void iupmotCBcaret (Widget w, XtPointer client_data, XtPointer call_data)
{
   IFnii cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;
   XmTextVerifyCallbackStruct* textverify = (XmTextVerifyCallbackStruct*)call_data;

   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;
   cb = (IFnii) IupGetCallback(n, "CARET_CB");
   if (cb)
   {
      int old_col, old_row, col, row=1;
      long int pos;

      pos = textverify->newInsert;

      if (type(n) == TEXT_ || (type(n) == LIST_ && iupCheck(n, "EDITBOX")==YES))
      {
        col = pos+1;
      }
      else /* MULTILINE_ */
      {
        long int linText, colText;
        char *str = XmTextGetString((Widget)handle(n));
        iupmotLincol( str, pos, &linText, &colText );
        row = linText;
        col = colText;
      }

      old_col = IupGetInt(n, "_IUPMOT_CARETCOL");
      old_row = IupGetInt(n, "_IUPMOT_CARETROW");
      if (row != old_row || col != old_col)
      {
        IupSetfAttribute(n, "_IUPMOT_CARETCOL", "%d", col);
        IupSetfAttribute(n, "_IUPMOT_CARETROW", "%d", row);

        iupmot_incallback = TRUE;
        if (cb(n, row, col) == IUP_CLOSE) 
          iupmot_exitmainloop = 1;
        iupmot_incallback = FALSE;
      }
   }
}

void iupmotCBbutton (Widget w, XtPointer client_data, XtPointer call_data)
{
   IFn cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;

   iupmotTipLeaveNotify();

   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;
   cb = (IFn) IupGetCallback(n,IUP_ACTION);
   if (cb)
   {
      iupmot_incallback = TRUE;
      if (cb(n) == IUP_CLOSE) 
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

void iupmotCBtoggle (Widget w, XtPointer client_data, XtPointer call_data)
{
   XmToggleButtonCallbackStruct *toggledata = 
      (XmToggleButtonCallbackStruct *)call_data;
   Iwidgetdata *d = NULL;
   Ihandle *n;
   IFni cb;

   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;

   cb = (IFni) IupGetCallback(n,IUP_ACTION);
   if (cb)
   {
      int check = toggledata->set;
      if (check == XmINDETERMINATE)
         check = -1;

      iupmot_incallback = TRUE;
      if (cb(n, check) == IUP_CLOSE) 
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

Ihandle *iupmotFindRadioOption (Ihandle *n)
{
   if (n == NULL) return NULL;

   if(type(n) == FRAME_ || type(n) == RADIO_)
   {
         return iupmotFindRadioOption(child(n));
   }
   else if(type(n) == HBOX_ || type(n) == VBOX_)
   {
         Ihandle *c, *op = 0;
         foreachchild(c, n) 
         {
            op = iupmotFindRadioOption(c);
            if (op) break;
         }
         return op;
   }
   else if(type(n) == ZBOX_)
   {
         return iupmotFindRadioOption( iupmotGetZboxValue(n) );
   }
   else if(type(n) == TOGGLE_)
   {
         if (handle(n))
         {
            unsigned char val;
            XtVaGetValues ((Widget)handle(n), XmNset, &val, NULL);
            if (val) return n;
         }
   }
   return NULL;
}

void iupmotCBradio (Widget w, XtPointer client_data, XtPointer call_data)
{
   Iwidgetdata *d = NULL;
   Ihandle *n;
   IFni cb;
   Ihandle *radio;
   Ihandle *option;

   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if (n == NULL) return;

   for (radio=parent(n);
        radio && type(radio) != RADIO_;
        radio=parent(radio)) ;

   XmToggleButtonSetState( (Widget)handle(n), False, False );
   option=iupmotFindRadioOption (radio);   /* find option ON != n*/
   XmToggleButtonSetState( (Widget)handle(n), True, False );

   if (option)                            /* turn OFF option */
   {
      XmToggleButtonSetState( (Widget)handle(option), False, False );
      cb = (IFni) IupGetCallback(option,IUP_ACTION);
      if (cb)
      {
         iupmot_incallback = TRUE;
         if (cb && cb(option, 0) == IUP_CLOSE) 
            iupmot_exitmainloop = 1;
         iupmot_incallback = FALSE;
      }
   }
   cb = (IFni) IupGetCallback(n,IUP_ACTION);
   if (cb)
   {
      iupmot_incallback = TRUE;
      if (cb && cb(n, 1) == IUP_CLOSE) 
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

void iupmotCBclose (Widget w, XtPointer client_data, XtPointer call_data)
{
   Ihandle *n = (Ihandle*)client_data;
   int (*cb)(Ihandle*);

   if (!n) return;
   cb = (int(*)(Ihandle*)) IupGetCallback(n,IUP_CLOSE_CB);
   if (cb)
   {
     int ret = cb(n);
     switch (ret)
     {
       case IUP_IGNORE:
           /* don't close the window */
         return;
       case IUP_CLOSE:
           /* close the application */
         iupmot_exitmainloop = 1;
         break;
       default:
           /* default: close the window */
         break;
     }
   }
   iupmotHideDialog(n);
}

void iupmotCBdialog (Widget w, XtPointer client_data, XtPointer call_data)
{
   Iwidgetdata *d = NULL;
   Ihandle *n;
 
   XtVaGetValues (w, 
     XmNuserData, &d, 
    NULL);
   if (!d) return;
   n = d->ihandle;
   if (!n) return;

   if (iupGetEnv(n,"_IUPMOT_DESTROY")!=NULL) return; 

   dlgResize(w, n);
}

static Ihandle *menuh = NULL;
static IFn menucb = NULL;

void iupmotCallMenu(void)
{
   if (menucb)
   {
      IFn cb = menucb;

      menucb = NULL;
      iupmot_incallback = TRUE;
      if (cb(menuh) == IUP_CLOSE) iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

void iupmotCBmenu (Widget w, XtPointer client_data, XtPointer call_data)
{
   IFn cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;
 
   if (XtClass(w)==xmToggleButtonWidgetClass)
   {
      XmToggleButtonCallbackStruct *tb =
        (XmToggleButtonCallbackStruct*) call_data;
      XmToggleButtonSetState(w,!tb->set,0);
   }

   if (iupmot_incallback)
     return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;
   if (!n) return;

   cb = (IFn) IupGetCallback(n,IUP_ACTION);
   if (cb)
   {
      /* nao pode chamar callback aqui, pois se a callback
         fechar o dialogo (pai do menu), da erro */
      menucb = cb;
      menuh = n;
   }
}

void iupmotCBarmmenu (Widget w, XtPointer client_data, XtPointer call_data)
{
   Icallback cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;
 
   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;
   if (!n) return;

   cb = IupGetCallback(n, IUP_HIGHLIGHT_CB);
   if (cb)
   {
     iupmot_incallback = 1;
     if (cb (n)==IUP_CLOSE) iupmot_exitmainloop = 1;
     iupmot_incallback = 0;
   }
}

void iupmotCBmapmenu (Widget w, XtPointer client_data, XtPointer call_data)
{
   Icallback cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;
 
   if (iupmot_incallback) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;
   if (!n) return;
   
   /* Motif returns the child of the SUBMENU_ */
   n  = IupGetParent(n);
   if (!n) return;

   cb = IupGetCallback(n, (char*)client_data);
   if (cb)
   {
     iupmot_incallback = 1;
     cb(n);                  /* do not handle IUP_CLOSE here */
     iupmot_incallback = 0;
   }
}

void iupmotCBresize(Widget w, XtPointer client_data, XtPointer call_data)
{
   Dimension width, height;
   IFnii cb;
   Iwidgetdata *d = NULL;
   Ihandle *n;

   if (!XtWindow(w)) return;

   XtVaGetValues (w, XmNuserData, &d,
                     XmNwidth, &width,
                     XmNheight, &height,
                     NULL);
   if (!d) return;
   n = d->ihandle;

   iupSetEnv(n,RESIZE_FIRST_TIME,"1");

   cb = (IFnii) IupGetCallback(n,IUP_RESIZE_CB);

   if (cb)
   {
      iupmot_incallback = TRUE;
      if (cb (n,width,height)==IUP_CLOSE)
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }

}

static void xrepaint(Widget w, XExposeEvent *ev)
{
   Iwidgetdata *d = NULL;
   Ihandle *n;
   IFnff cb;

   if (!XtWindow(w)) return;

   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;

   if ( iupTableGet(env(n),RESIZE_FIRST_TIME) == NULL )
      iupmotCBresize( w, NULL, NULL );

   cb = (IFnff)IupGetCallback(n,IUP_ACTION);
   if (cb)
   {
      float x = IupGetFloat(n, IUP_POSX);
      float y = IupGetFloat(n, IUP_POSY);
 
      iupmot_incallback = TRUE;
      if (cb (n,x,y)==IUP_CLOSE)
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

void iupmotCBrepaint  (Widget w, XtPointer client_data, XtPointer call_data)
{
   static int removing = 0;
   XExposeEvent *ev = (XExposeEvent*) ((XmDrawingAreaCallbackStruct*)call_data)->event;

   if (ev->count != 0 || removing != 0)
      return;

   removing = 1;
   {
      XEvent event;
      while (XCheckWindowEvent(iupmot_display, XtWindow(w), ExposureMask, &event))
         XtDispatchEvent(&event);
   }
   removing = 0;

   xrepaint(w, ev);
}

static void xinput(Widget w, XEvent *event)
{
   Iwidgetdata *d = NULL;
   Ihandle *n;
   static char report[12];
   static Time last = 0;
   unsigned long elapsed;
   
   XtVaGetValues (w, XmNuserData, &d, NULL);
   if (!d) return;
   n = d->ihandle;
   if (!n) return;

   switch (event->type)
   {
      case MotionNotify:
      {
         IFniis cb = (IFniis)IupGetCallback(n,IUP_MOTION_CB);
         if (cb)
         {
            XMotionEvent *ev = (XMotionEvent*)event;
            makeStateString(ev->state, 0, report);
            iupmot_incallback = TRUE;
            if ( cb (n, ev->x, ev->y, report)==IUP_CLOSE)
               iupmot_exitmainloop = 1;
            iupmot_incallback = FALSE;
         }
      }
      break;

      case ButtonPress:
         /* Clicked element should receive focus (needed by
          * GLCANVAS but should not bring any harm to other
          * elements.
          */
         XmProcessTraversal(w, XmTRAVERSE_CURRENT);
         /* break missing on purpose... */
      case ButtonRelease:
      {
         IFniiiis cb = (IFniiiis) IupGetCallback(n,IUP_BUTTON_CB);
         XButtonEvent *ev = (XButtonEvent*)event;
         if ( ev->button!=Button1 &&
              ev->button!=Button2 &&
              ev->button!=Button3 &&
              ev->button!=Button4 &&
              ev->button!=Button5 ) 
              return;

         if (cb && ev->button!=Button4 && ev->button!=Button5)
         {
            int b = (ev->button==Button1)?'1':
                   ((ev->button==Button2)?'2':'3');
            makeStateString(ev->state, ev->button, report);

      /* Double/Single Click */
            if (ev->type==ButtonPress)
            {
               elapsed = ev->time - last;
               last = ev->time;
               if (elapsed <= XtGetMultiClickTime(iupmot_display))
                  strcat(report,"D");   /* Double_click */
               else
                  strcat(report,"S");   /* Single_click */
            }

            iupmot_incallback = TRUE;
            if ( cb (n, b, (ev->type==ButtonPress),
                     ev->x, ev->y, report)==IUP_CLOSE)
               iupmot_exitmainloop = 1;
            iupmot_incallback = FALSE;
         }

         if (ev->button==Button4 || ev->button==Button5)
         {                                             
            typedef int (*mwF)(Ihandle*, float, int, int, char*);
            mwF wcb = (mwF)IupGetCallback(n, "WHEEL_CB");
            if (wcb)
            {
              int delta = ev->button==Button4? 1: -1;
              makeStateString(ev->state, ev->button, report);

              iupmot_incallback = TRUE;
              if ( wcb (n, (float)delta, ev->x, ev->y, report)==IUP_CLOSE)
                iupmot_exitmainloop = 1;
              iupmot_incallback = FALSE;
            }
            else
            {
              IFniff scb = (IFniff)IupGetCallback(n,IUP_SCROLL_CB);
              if (scb)
              {
                float posy = IupGetFloat(n,IUP_POSY);
                int delta = ev->button==Button4? 1: -1;
                posy -= delta*IupGetFloat(n, IUP_DY)/10.0f;
                IupSetfAttribute(n, IUP_POSY, "%g", posy);
                iupmot_incallback = TRUE;
                scb (n, IUP_SBDRAGV,(float)IupGetFloat(n,IUP_POSX),(float)IupGetFloat(n,IUP_POSY));
                iupmot_incallback = FALSE;
              }
            }
         }
      }
      break;
   }
}

void iupmotCBinput( Widget w, XtPointer client_data, XtPointer call_data )
{
   xinput( w, ((XmDrawingAreaCallbackStruct*)call_data)->event );
}

void iupmotCBmotion(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
   xinput(w, ev);
}

void iupmotCBscrollbar( Widget w, XtPointer client_data, XtPointer call_data )
{
  Widget canvas = XtNameToWidget( XtParent(w), "canvas" );
  Iwidgetdata *d = NULL;
  Ihandle *n;
  IFniff cb;

  /* pega o handle iup do campo userdata do canvas associado ao sb */
  XtVaGetValues (canvas, XmNuserData, &d, NULL);
  if (!d) return;
  n = d->ihandle;

  cb = (IFniff)IupGetCallback(n,IUP_SCROLL_CB);
  if (cb)
  {
    iupmot_incallback = TRUE;
    cb (n, (int)client_data,IupGetFloat(n,IUP_POSX),IupGetFloat(n,IUP_POSY));
    iupmot_incallback = FALSE;
  }
  else
  {
    XClearArea( iupmot_display, XtWindow(canvas), 0, 0, 0, 0, True );
  }
}

static void callCanvasFocusCb(Ihandle *n, int focus)
{
  IFni cb = (IFni)IupGetCallback(n, "FOCUS_CB");
  if (cb)
  {
    iupmot_incallback = TRUE;
    if (cb(n, focus) == IUP_CLOSE)
        iupmot_exitmainloop = 1;
    iupmot_incallback = FALSE;
  }
}

void iupmotCBfocus(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
   Icallback cb;
   Ihandle *n = (Ihandle*)data;
 
   if (ev->xfocus.detail != NotifyAncestor) return;

   if (iupGetEnv(n,"_IUPMOT_DESTROY")!=NULL) return; 

   if (ev->type == FocusIn)
   {
      iupmot_focus = n;
      if (iupmot_post_killfocus)
      {
        /* ComboBox has two controls: a list and a editbox. 
           Both have getfocus and killfocus */
        if (iupmot_post_killfocus != n)
        {
          /* if from edit/list to a different control call the missing callback */
          cb = IupGetCallback(iupmot_post_killfocus,IUP_KILLFOCUS_CB);
          if (cb)
          {
            iupmot_incallback = TRUE;
            if (cb(iupmot_post_killfocus) == IUP_CLOSE)
                iupmot_exitmainloop = 1;
            iupmot_incallback = FALSE;
          }
          if (type(n) == CANVAS_) callCanvasFocusCb(n, 0);

          iupmot_post_killfocus = NULL;
        }
        else
        {
          /* if from edit to list or vice-versa do not call any callbacks */
          iupmot_post_killfocus = NULL;
          return;
        }
      }

      cb = IupGetCallback(n,IUP_GETFOCUS_CB);
      if (cb)
      {
         iupmot_incallback = TRUE;
         if (cb(n) == IUP_CLOSE)
            iupmot_exitmainloop = 1;
         iupmot_incallback = FALSE;
      }
      if (type(n) == CANVAS_) callCanvasFocusCb(n, 1);
   }
   else if (ev->type == FocusOut)
   {
      iupmot_focus = 0;
      if (type(n) == TEXT_) /* tem mascara */
      {
         ITextInfo *info;
         Iwidgetdata *wd = NULL;
         XtVaGetValues (w, XmNuserData, &wd, NULL);
         if (!wd) return;
         info = (ITextInfo*)wd->data;
         if (info && info->status == 2) callMaskCallback(n, 1);
      }
      else if (type(n) == LIST_ && (iupCheck(n, IUP_DROPDOWN)==YES ||
                                   iupCheck(n, "EDITBOX")==YES))
      {
        iupmot_post_killfocus = n;
        return;
      }

      cb = IupGetCallback(n,IUP_KILLFOCUS_CB);
      if (cb)
      {
         iupmot_incallback = TRUE;
         if (cb(n) == IUP_CLOSE)
            iupmot_exitmainloop = 1;
         iupmot_incallback = FALSE;
      }

      if (type(n) == CANVAS_) callCanvasFocusCb(n, 0);
   }
}

static void defaultbuttons( Ihandle* n, int key, XEvent *ev )
{
  if ((type(n)!=MULTILINE_ && key==K_CR) || key==K_ESC) 
  {
    char *button_name = NULL;
    char* default_but = key==K_CR? IUP_DEFAULTENTER: IUP_DEFAULTESC;
    Ihandle *dialog = IupGetDialog(n);

    if (key == K_CR)
    {
      Ihandle* focus = IupGetFocus();
      if (focus && type(focus) == BUTTON_)
      {
        XtCallActionProc((Widget)handle(focus), "ArmAndActivate", ev, 0, 0 );
        return;
      }
    }

    button_name = IupGetAttribute(dialog, default_but);
    if(button_name)
    {
      Ihandle* bt = IupGetHandle(button_name);
      if(bt && type(bt) == BUTTON_ && IupGetDialog(bt)==dialog)
        XtCallActionProc( (Widget) handle(bt), "ArmAndActivate", ev, 0, 0 );
    }
  }
}

/* Discards keyrepeat by removing the keypress event from the queue.
 * The pair keyrelease/keypress is always put together in the queue,
 * by removing the keypress, we only worry about keyrelease. In case
 * of a keyrelease, we ignore it if the next event is a keypress (which
 * means repetition. Otherwise it is a real keyrelease.
 *
 * Returns 1 if the keypress is found in the queue and 0 otherwise.
 */
static int discardkeypressrepeat(XEvent *ev)
{
  XEvent ahead;
  if (XEventsQueued(iupmot_auxdisplay, QueuedAfterReading))
  {
    XPeekEvent(iupmot_auxdisplay, &ahead);
    if (ahead.type == KeyPress && ahead.xkey.window == ev->xkey.window
        && ahead.xkey.keycode == ev->xkey.keycode && ahead.xkey.time == ev->xkey.time)
    {
      /* Pop off the repeated KeyPress and ignore */
      XNextEvent(iupmot_auxdisplay, ev);
      /* Ignore the auto repeated KeyRelease/KeyPress pair */
      return 1;
    }
  }
  /* No KeyPress found */
  return 0;
}

static void call_k_any(Ihandle *p, int c)
{
  char *attr = iupKeyEncode(c);
  for (; p; p=parent(p))
  {
    IFni cb = NULL;
    if (attr)
      cb = (IFni)IupGetCallback(p,attr);
    if (!cb)
      cb = (IFni)IupGetCallback(p,"K_ANY");

    if (cb)
    {
      int result;
      iupmot_incallback = TRUE;
      result = cb(p, c);
      iupmot_incallback = FALSE;
      if (result == IUP_CLOSE) iupmot_exitmainloop = 1;
      if (result != IUP_CONTINUE) return;
    }
  }
}

static void call_keypress_cb(Ihandle *p, int code, int press)
{
  IFnii cb = (IFnii)IupGetCallback(p, "KEYPRESS_CB");
  if (cb)
  {
    int result;
    iupmot_incallback = TRUE;
    result = cb(p, code, press);
    iupmot_incallback = FALSE;
    if (result == IUP_CLOSE) 
      iupmot_exitmainloop = 1;
  }
}

void iupmotCBkeyrelease(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
  Ihandle *p = (Ihandle*)data;
  int code = iupmotKeyGetCode((XKeyEvent*)ev);
  if (code == 0) 
    return;

  if(discardkeypressrepeat(ev))
  {
    /* call k_any because it was removed from the queue */
    call_k_any(p, code);
    call_keypress_cb(p, code, 1);
    defaultbuttons(p, code, ev);
    return;
  }
  
  call_keypress_cb(p, code, 0);
}

void iupmotCBkeypress(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
  Ihandle *p = (Ihandle*)data;
  int code = iupmotKeyGetCode((XKeyEvent*)ev);
  if (code == 0) 
    return;
  call_k_any(p, code);
  if (type(p)==CANVAS_) call_keypress_cb(p, code, 1);
  defaultbuttons(p, code, ev);
}

static void set_shadow(Ihandle* n, int shadow)
{
  XtVaSetValues ((Widget)handle(n), 
                  XmNshadowThickness, shadow,
                  NULL);
}

void iupmotCBenterleave(Widget w, XtPointer data, XEvent *ev, Boolean *cont)
{
   Icallback cb = NULL;
   Ihandle *n = (Ihandle*)data;;

   if (ev->type == EnterNotify)
   {
      iupmotTipEnterNotify( w, n );

      cb = IupGetCallback(n,IUP_ENTERWINDOW_CB);

      if (type(n) == BUTTON_)
      {
        if (iupCheck(n, "FLAT")==YES)
          set_shadow(n, 2);
      }
   }
   else  if (ev->type == LeaveNotify)
   {
      iupmotTipLeaveNotify();

      cb = IupGetCallback(n,IUP_LEAVEWINDOW_CB);

      if (type(n) == BUTTON_)
      {
        if (iupCheck(n, "FLAT")==YES)
          set_shadow(n, 0);
      }
   }

   if (cb)
   {
      iupmot_incallback = TRUE;
      if (cb(n) == IUP_CLOSE)
         iupmot_exitmainloop = 1;
      iupmot_incallback = FALSE;
   }
}

