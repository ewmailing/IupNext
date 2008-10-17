/** \file
 * \brief iupval control
 * Calls the user callbacks
 *
 * See Copyright Notice in iup.h
 * $Id: iupval.c,v 1.1 2008-10-17 06:19:56 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <iup.h>
#include <iupcpi.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include <iupcompat.h>
#include "iupcdutil.h"
#include "iupval.h"
#include "icontrols.h"

#define DEFAULTVSIZE "28x124"
#define DEFAULTHSIZE "124x28"
#define VDEFAULT_STEP 0.01
#define VDEFAULT_PAGESTEP 0.1
#define HANDLER_LONG  22  /* handler long side */
#define HANDLER_SHORT 16  /* handler short side */
#define TRAIL          4  /* with of the trail */
#define TICK           3  /* size of the tick mark */
#define TRAIL_START    (HANDLER_LONG/2+1)
#define TRAIL_END(_s)  (_s-1-HANDLER_LONG/2-1)
#define TRAIL_LONG(_s) (_s-1-HANDLER_LONG-1)

typedef int (*Valaplcb)(Ihandle *,double);

typedef struct _Tval
{
  /* Values */
  int w,h;
  cdCanvas *cddbuffer;
  cdCanvas *cdcanvas;
  long bgcolor,
       light_shadow,
       mid_shadow,
       dark_shadow;
  int moving,
      has_focus, 
      show_ticks;
  double val,
         step,
         pagestep,
         vmin,
         vmax;

  unsigned char* himage;
  int himage_w;
  int himage_h;
  long int* himage_colors;
  int himage_numcolors;

  unsigned char* himageinactive;
  int himageinactive_w;
  int himageinactive_h;
  long int* himageinactive_colors;
  int himageinactive_numcolors;

  /* Internal methods */
  void (*drawval)(struct _Tval *, Ihandle*);
  void (*setval)(Ihandle*,struct _Tval*,int,int);
} Tval;

static void updateBgColor(long int* colors, int num_colors, long bgcolor)
{
  int c;
  for (c = 0; c < num_colors; c++)
  {
    if (*colors == -1)
      *colors = bgcolor;
    colors++;
  }
}

static void setimage(char* name, unsigned char** image_out, long int** color_out, 
                    int* w_out, int* h_out, int* numcolors_out)
{
  int x, y, c, width, height, max_color = 0;
  unsigned char* image_data;
  Ihandle *img = IupGetHandle(name);    

  *w_out = width = IupGetInt(img, IUP_WIDTH);
  *h_out = height = IupGetInt(img, IUP_HEIGHT);
  image_data = iupGetImageData(img);

  *image_out = (unsigned char*) realloc(*image_out, width*height*sizeof(unsigned char));
  *color_out = (long int*) realloc(*color_out, 256*sizeof(long int));

  for(y = height-1 ; y >=0 ; y--) 
  {
    for(x = 0 ; x < width ; x++)  
    {
      (*image_out)[x+width*y] = *image_data;
      if (*image_data > max_color)
        max_color = *image_data;
      image_data++;
    }
  }

  max_color++;
  *numcolors_out = max_color;

  for(c = 0 ; c < max_color; c++) 
  {
    char colorstr[10];
    char *value;
    sprintf(colorstr, "%d", c);
    value = IupGetAttribute(img,colorstr);      
    if (iupStrEqualNoCase(value,"BGCOLOR")) 
    {   
      (*color_out)[c] = -1;
    }
    else
    {
      unsigned int r = 0, g = 0, b = 0;
      iupGetRGB(value,&r,&g,&b);      
      (*color_out)[c] = cdEncodeColor((unsigned char)r,(unsigned char)g,(unsigned char)b);
    }
  }
}

static void beginimage(Tval *v, Ihandle* self)
{
  v->bgcolor = cdIupConvertColor(iupGetBgColor(self));
  cdIupCalcShadows(v->bgcolor, &v->light_shadow, &v->mid_shadow, &v->dark_shadow);

  if (!IupGetInt(self, "ACTIVE") && !v->himage) 
    v->light_shadow = v->mid_shadow;

  if (v->himage)
    updateBgColor(v->himage_colors, v->himage_numcolors, v->bgcolor);
  if (v->himageinactive)
    updateBgColor(v->himageinactive_colors, v->himageinactive_numcolors, v->bgcolor);

  cdCanvasBackground(v->cddbuffer,v->bgcolor);
  cdCanvasClear(v->cddbuffer);
}

static void putimage(Tval *v, Ihandle* self)
{
  cdCanvasFlush(v->cddbuffer);

  if (v->has_focus)
    cdIupDrawFocusRect(self, v->cdcanvas, 0, 0, v->w-1, v->h-1);
}

static int resize_cb(Ihandle *self);

static void drawval(Tval *v, Ihandle* self)
{
  if (v->cddbuffer == NULL)
    resize_cb(self);

  beginimage(v,self);
  v->drawval(v, self);
  putimage(v, self);
}

static void vdrawtick(Tval *v, int y)
{
  cdIupDrawHorizSunkenMark(v->cddbuffer, 1, 1+TICK, y, v->light_shadow, v->dark_shadow);
  cdIupDrawHorizSunkenMark(v->cddbuffer, v->w-2-TICK, v->w-2, y, v->light_shadow, v->dark_shadow);
}

static void hdrawtick(Tval *v, int x)
{
  cdIupDrawVertSunkenMark(v->cddbuffer, x, 1, 1+TICK, v->light_shadow, v->dark_shadow);
  cdIupDrawVertSunkenMark(v->cddbuffer, x, v->h-2-TICK, v->h-2, v->light_shadow, v->dark_shadow);
}

static void vdrawticks(Tval *v)
{
  int i;
  for (i = 0; i < v->show_ticks-1; i++)
  {
    vdrawtick(v, (i*(TRAIL_LONG(v->h)-2))/(v->show_ticks-1) + TRAIL_START+1);
  }

  vdrawtick(v, TRAIL_END(v->h));
}

static void hdrawticks(Tval *v)
{
  int i;
  for (i = 0; i < v->show_ticks-1; i++)
  {
    hdrawtick(v, (i*(TRAIL_LONG(v->w)-2))/(v->show_ticks-1) + TRAIL_START+1);
  }

  hdrawtick(v, TRAIL_END(v->w));
}

static void vdrawval(Tval *v, Ihandle* self)
{
  int y, x = (v->w-TRAIL)/2;
  double vn = (v->val-v->vmin)/(v->vmax-v->vmin);

  /* trail */
  cdIupDrawSunkenRect(v->cddbuffer, x, TRAIL_START, x+TRAIL-1, TRAIL_END(v->h)+1, 
                      v->light_shadow, v->mid_shadow, v->dark_shadow);

  if (v->show_ticks > 2)
    vdrawticks(v);

  if (IupGetInt(self, "ACTIVE") && v->himage) 
  {
    x = (v->w - v->himage_w)/2;
    y = (int)(vn*(TRAIL_LONG(v->h)-2) + TRAIL_START+1);
    cdCanvasPutImageRectMap(v->cddbuffer,v->himage_w, v->himage_h, v->himage, v->himage_colors,
                      x, y-v->himage_h/2, 0,0,0,0,0,0);
  } 
  else if (!IupGetInt(self, "ACTIVE") && v->himageinactive) 
  {
    x = (v->w - v->himageinactive_w)/2;
    y = (int)(vn*(TRAIL_LONG(v->h)-2) + TRAIL_START+1);
    cdCanvasPutImageRectMap(v->cddbuffer,v->himageinactive_w, v->himageinactive_h, v->himageinactive, 
                      v->himageinactive_colors, x, y-v->himageinactive_h/2, 0,0,0,0,0,0);
  } 
  else
  {
    /* handler border */
    x = (v->w-HANDLER_SHORT)/2;
    y = (int)(vn*(TRAIL_LONG(v->h)-2) + TRAIL_START+1);
    cdIupDrawRaisenRect(v->cddbuffer, x, y-HANDLER_LONG/2-1, x+HANDLER_SHORT-1, y+HANDLER_LONG/2, 
                        v->light_shadow, v->mid_shadow, v->dark_shadow);

    /* handler background */
    cdCanvasForeground(v->cddbuffer, v->bgcolor );
    cdCanvasBox(v->cddbuffer,x+1, x+HANDLER_SHORT-1-2, y-HANDLER_LONG/2+1, y+HANDLER_LONG/2-1);

    /* handler sunken mark */
    cdIupDrawHorizSunkenMark(v->cddbuffer, x+2, x+HANDLER_SHORT-4, y, v->light_shadow, v->dark_shadow);
  }
}

static void hdrawval(Tval *v, Ihandle* self)
{
  int x, y = (v->h-TRAIL)/2;
  double vn = (v->val-v->vmin)/(v->vmax-v->vmin);

  /* trail */
  cdIupDrawSunkenRect(v->cddbuffer, TRAIL_START, y, TRAIL_END(v->w)+1, y+TRAIL-1, 
                      v->light_shadow, v->mid_shadow, v->dark_shadow);
  if (v->show_ticks > 2)
    hdrawticks(v);

  if (IupGetInt(self, "ACTIVE") && v->himage) 
  {
    y = (v->h - v->himage_h)/2+1;
    x = (int)(vn*(TRAIL_LONG(v->w)-2) + TRAIL_START+1);
    cdCanvasPutImageRectMap(v->cddbuffer,v->himage_w, v->himage_h, v->himage, v->himage_colors,
                      x-v->himage_w/2, y, 0,0,0,0,0,0);
  } 
  else if (!IupGetInt(self, "ACTIVE") && v->himageinactive) 
  {
    y = (v->h - v->himageinactive_h)/2+1;
    x = (int)(vn*(TRAIL_LONG(v->w)-2) + TRAIL_START+1);
    cdCanvasPutImageRectMap(v->cddbuffer,v->himageinactive_w, v->himageinactive_h, v->himageinactive, v->himageinactive_colors,
                      x-v->himageinactive_w/2, y, 0,0,0,0,0,0);
  } 
  else 
  {
    /* handler border */
    y = (v->h-HANDLER_SHORT)/2;
    x = (int)(vn*(TRAIL_LONG(v->w)-2) + TRAIL_START+1);
    cdIupDrawRaisenRect(v->cddbuffer, x-HANDLER_LONG/2-1, y, x+HANDLER_LONG/2, y+HANDLER_SHORT-1, 
                        v->light_shadow, v->mid_shadow, v->dark_shadow);

    /* handler background */
    cdCanvasForeground(v->cddbuffer, v->bgcolor );
    cdCanvasBox(v->cddbuffer,x-HANDLER_LONG/2, x+HANDLER_LONG/2-2, y+1, y+HANDLER_SHORT-1-2);

    /* handler sunken mark */
    cdIupDrawVertSunkenMark(v->cddbuffer, x, y+3, y+HANDLER_SHORT-3, v->light_shadow, v->dark_shadow);
  }
}

static void crop_value(Tval *v)
{
  if (v->val > v->vmax) v->val=v->vmax;
  else if (v->val < v->vmin) v->val=v->vmin;
}

static void vsetval(Ihandle* self, Tval *v, int x, int y)
{
  double vn;
  (void)x;
  y = cdIupInvertYAxis(y, v->h);
  vn = ((double)(y-TRAIL_START))/((double)TRAIL_LONG(v->h));
  v->val = vn*(v->vmax-v->vmin) + v->vmin;
  crop_value(v);
  drawval(v, self);
}

static void hsetval(Ihandle* self, Tval *v, int x, int y)
{
  double vn;
  (void)y;
  vn = ((double)(x-TRAIL_START))/((double)TRAIL_LONG(v->w));
  v->val = vn*(v->vmax-v->vmin) + v->vmin;
  crop_value(v);
  drawval(v, self);
}

static int motion_cb(Ihandle *self,int x,int y,char *r)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  Valaplcb cb;

  if (!isbutton1(r)) return IUP_DEFAULT;
  if (!v->moving) return IUP_DEFAULT;
  if (!IupGetInt(self, "ACTIVE")) return IUP_DEFAULT;

  v->setval(self,v,x,y);

  cb = (Valaplcb) IupGetCallback(self, ICTL_MOUSEMOVE_CB);
  if (cb) cb(self,v->val);

  return IUP_DEFAULT;
}

static int button_cb(Ihandle *self,int b,int m,int x,int y)
{
  char* cb_name;
  Valaplcb cb;
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  if (b!=IUP_BUTTON1) return IUP_DEFAULT;
  if (!IupGetInt(self, "ACTIVE")) return IUP_DEFAULT;

  if (m) 
  {
    v->setval(self, v, x, y);
    v->moving = 1;

    cb_name = ICTL_BUTTON_PRESS_CB;
  }
  else
  {
    if (!v->moving) return IUP_DEFAULT;

    drawval(v, self);
    v->moving = 0;

    cb_name = ICTL_BUTTON_RELEASE_CB;
  }

  cb = (Valaplcb) IupGetCallback(self, cb_name);
  if (cb) cb(self,v->val);

  return IUP_DEFAULT;
}

static void createcanvas(Tval *v, Ihandle *self)
{
  if (!v->cdcanvas)
    v->cdcanvas = cdCreateCanvas(CD_IUP, self);

  if (v->cdcanvas)
  {
    cdCanvasActivate(v->cdcanvas);
    v->cddbuffer = cdCreateCanvas(CD_DBUFFER, v->cdcanvas);
  }
}

static int resize_cb(Ihandle *self)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");

  if (!v->cddbuffer)
    createcanvas(v, self);

  if (!v->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(v->cddbuffer);
  cdCanvasGetSize(v->cddbuffer,&v->w,&v->h,NULL,NULL);

  return IUP_DEFAULT;
}

static int repaint_cb(Ihandle *self)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  if(!v) return IUP_DEFAULT;

  if (!v->cddbuffer)
    createcanvas(v, self);

  if (!v->cddbuffer)
    return IUP_DEFAULT;

  drawval(v, self);

  return IUP_DEFAULT;
}

static int focus_cb (Ihandle * self, int focus)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  if (v == NULL)
    return IUP_DEFAULT;
  if (!IupGetInt(self, "ACTIVE")) return IUP_IGNORE;

  v->has_focus = focus;
  drawval(v, self);

  return IUP_DEFAULT;
}

static int keypress_cb(Ihandle *self, int c, int press)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  char* cb_name;
  Valaplcb cb;

  if (v == NULL) return IUP_DEFAULT;
  if (!IupGetInt(self, "ACTIVE")) return IUP_DEFAULT;

  if (c != K_LEFT && c != K_UP &&
      c != K_PGDN && c != K_PGUP  &&
      c != K_RIGHT && c != K_DOWN  &&
      c != K_END && c != K_HOME &&
      c != K_sLEFT && c != K_sUP &&
      c != K_sPGDN && c != K_sPGUP  &&
      c != K_sRIGHT && c != K_sDOWN  &&
      c != K_sEND && c != K_sHOME)
    return IUP_DEFAULT;

  if (press)
  {
    switch(c)
    {
    case K_sHOME:
    case K_HOME:
      v->val = v->vmax;
      break;
    case K_sEND:
    case K_END:
      v->val = v->vmin;
      break;
    case K_sLEFT:
    case K_sDOWN:
    case K_LEFT:
    case K_DOWN:
      v->val -= v->step*(v->vmax-v->vmin);
      crop_value(v);
      break;
    case K_sRIGHT:
    case K_sUP:
    case K_RIGHT:
    case K_UP:
      v->val += v->step*(v->vmax-v->vmin);
      crop_value(v);
      break;
    case K_sPGDN:
    case K_PGDN:
      v->val -= v->pagestep*(v->vmax-v->vmin);
      crop_value(v);
      break;
    case K_sPGUP:
    case K_PGUP:
      v->val += v->pagestep*(v->vmax-v->vmin);
      crop_value(v);
      break;
    }

    if (fabs(v->val-v->vmin) < 0.9*v->step*(v->vmax-v->vmin))
      v->val = v->vmin;

    if (v->moving)
      cb_name = ICTL_MOUSEMOVE_CB;
    else
    {
      v->moving = 1;
      cb_name = ICTL_BUTTON_PRESS_CB;
    }
  }
  else
  {
    v->moving = 0;
    cb_name = ICTL_BUTTON_RELEASE_CB;
  }

  drawval(v, self);

  cb = (Valaplcb) IupGetCallback(self, cb_name);
  if (cb) cb(self,v->val);

  return IUP_DEFAULT;
}

static int wheel_cb(Ihandle *self, float delta)
{
  Tval *v=(Tval *)iupGetEnv(self,"_IUPVAL_DATA");
  Valaplcb cb;

  if (v == NULL) return IUP_DEFAULT;
  if (!IupGetInt(self, "ACTIVE")) return IUP_DEFAULT;

  v->val += ((double)delta)*v->step*(v->vmax-v->vmin);
  crop_value(v);

  if (fabs(v->val-v->vmin) < 0.9*v->step*(v->vmax-v->vmin))
    v->val = v->vmin;

  drawval(v, self);

  cb = (Valaplcb) IupGetCallback(self, ICTL_BUTTON_PRESS_CB);
  if (cb) cb(self,v->val);

  cb = (Valaplcb) IupGetCallback(self, ICTL_BUTTON_RELEASE_CB);
  if (cb) cb(self,v->val);

  return IUP_DEFAULT;
}

static void valuatormap(Ihandle *h, Ihandle *parent)
{
  char *tmp;
  Tval *v=(Tval *)iupGetEnv(h,"_IUPVAL_DATA");

  /* Faz o mapeamento do CANVAS */
  iupCpiDefaultMap(h,parent);

  tmp = iupGetEnv(h, IUP_MIN);
  if(tmp == NULL)
    v->vmin=0;
  else
  {
    v->vmin=atof(tmp);
    v->val = v->vmin;
  }

  tmp = iupGetEnv(h, IUP_MAX);
  if(tmp == NULL)
    v->vmax=1;
  else
    v->vmax=atof(tmp);

  tmp = iupGetEnv(h, IUP_VALUE);
  if(tmp == NULL)
    v->val=0;
  else
    v->val=atof(tmp);
}

static void valuatordestroy(Ihandle *h)
{
  Tval *v=(Tval *)iupGetEnv(h,"_IUPVAL_DATA");
  if (v==NULL) return;

  if (v->cddbuffer) cdKillCanvas(v->cddbuffer);
  if (v->cdcanvas) cdKillCanvas(v->cdcanvas);

  if (v->himage) free(v->himage);
  if (v->himage_colors) free(v->himage_colors);
  if (v->himageinactive) free(v->himageinactive);
  if (v->himageinactive_colors) free(v->himageinactive_colors);

  free(v);

  iupSetEnv(h, "_IUPVAL_DATA", NULL);
}

static char* valuatorgetattr(Ihandle *h, char *attr)
{
  static char str[20];

  if(iupStrEqual(attr,IUP_VALUE))
  {
    Tval *v=(Tval *)iupGetEnv(h,"_IUPVAL_DATA");
    if (v) 
    {
      sprintf( str, "%f", v->val );
      return str;
    }
  }
  return iupCpiDefaultGetAttr(h, attr);
}

static void valuatorsetattr(Ihandle *h, char *attr, char *value)
{
  Tval *v=(Tval *)iupGetEnv(h,"_IUPVAL_DATA");

  if (v==NULL) 
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  if(iupStrEqual(attr,IUP_VALUE))
  {
    if(value==NULL) /* reseta para default */
      v->val = 0;
    else
      v->val = atof(value);
    crop_value(v);
  }
  else if(iupStrEqual(attr,"SHOWTICKS"))
  {
    if(value==NULL) /* reseta para default */
      v->show_ticks = 0;
    else
      v->show_ticks = atoi(value);
  }
  else if(iupStrEqual(attr, IUP_MIN)) 
  {
    if(value==NULL) /* reseta para default */
      v->vmin = 0;
    else
      v->vmin = atof(value);
    crop_value(v);
  }
  else if(iupStrEqual(attr, "STEP")) 
  {
    if(value==NULL) /* reseta para default */
      v->step = VDEFAULT_STEP;
    else
      v->step = atof(value);
  }
  else if(iupStrEqual(attr, "PAGESTEP")) 
  {
    if(value==NULL) /* reseta para default */
      v->pagestep = VDEFAULT_PAGESTEP;
    else
      v->pagestep = atof(value);
  }
  else if(iupStrEqual(attr, IUP_MAX))
  {
    if(value==NULL) /* reseta para default */
      v->vmax = 1;
    else
      v->vmax = atof(value);
    crop_value(v);
  }
  else if(iupStrEqual(attr, "TYPE"))
  {
    /* define comportamento do canvas dependente da direcao do valuator */
    if(iupStrEqualNoCase(value, ICTL_VERTICAL))
    {
      iupSetEnv (h, "TYPE", ICTL_VERTICAL);
      iupSetEnv (h, IUP_RASTERSIZE, DEFAULTVSIZE);
      v->drawval = vdrawval;
      v->setval  = vsetval;
    }
    else
    {
      iupSetEnv (h, "TYPE", ICTL_HORIZONTAL);
      iupSetEnv (h, IUP_RASTERSIZE, DEFAULTHSIZE);
      v->drawval = hdrawval;
      v->setval  = hsetval;
    }
  } 
  else if(iupStrEqual(attr, "HANDLER_IMAGE"))
  {
    if (value==NULL) 
    {
      if (v->himage) 
      {
        free(v->himage);
        free(v->himage_colors);
        v->himage = NULL;
        v->himage_colors = NULL;
      }
    } 
    else 
    {
      setimage(value, &(v->himage), &(v->himage_colors), &(v->himage_w), &(v->himage_h), &(v->himage_numcolors));
    }
  } 
  else if(iupStrEqual(attr, "HANDLER_IMAGE_INACTIVE"))
  {
    if (value==NULL) 
    {
      if (v->himageinactive) 
      {
        free(v->himageinactive);
        free(v->himageinactive_colors);
        v->himageinactive = NULL;
        v->himageinactive_colors = NULL;
      }
    } 
    else
    {
      setimage(value, &(v->himageinactive), &(v->himageinactive_colors), 
               &(v->himageinactive_w), &(v->himageinactive_h), &(v->himageinactive_numcolors));
    }
  }

  iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */

  if (IupGetAttribute(h, IUP_WID) != NULL && v->cddbuffer != NULL)
    drawval(v, h);
}

static Ihandle *valuatorcreate(Iclass *ic, Ihandle **params)
{
  Tval *v=NULL;
  Ihandle *self = IupCanvas(NULL);
  char *type = params? (char*) params[0] : NULL;
  (void)ic;

  v=(Tval*)malloc (sizeof(Tval));
  memset(v, 0, sizeof(Tval));

  /* hangs valuator data in canvas */
  iupSetEnv (self,"_IUPVAL_DATA",(char*)v);
  iupSetEnv (self,IUP_EXPAND, IUP_NO);
  iupSetEnv (self,IUP_BORDER, IUP_NO);
  iupSetEnv (self,IUP_SCROLLBAR, IUP_NO);

  IupSetCallback(self,"RESIZE_CB",(Icallback)resize_cb);
  IupSetCallback(self,"ACTION",(Icallback)repaint_cb);
  IupSetCallback(self,"BUTTON_CB",(Icallback)button_cb);
  IupSetCallback(self,"MOTION_CB",(Icallback)motion_cb);
  IupSetCallback(self,"FOCUS_CB",(Icallback)focus_cb);
  IupSetCallback(self,"WHEEL_CB",(Icallback)wheel_cb);
  IupSetCallback(self,"KEYPRESS_CB",(Icallback)keypress_cb);

  v->vmax = 1;
  v->step = VDEFAULT_STEP;
  v->pagestep = VDEFAULT_PAGESTEP;
  v->himage = NULL;
  v->himage_colors = NULL;
  v->himageinactive = NULL;
  v->himageinactive_colors = NULL;

  if (type)
    valuatorsetattr(self, "TYPE", type);
  else
    valuatorsetattr(self, "TYPE", ICTL_HORIZONTAL);

  return self;
}

Ihandle *IupVal(const char *type)
{
  return IupCreatep("val", (void*)type, NULL);
}

void IupValOpen(void)
{
  Iclass* ICValuator = iupCpiCreateNewClass("val" ,"s");

  iupCpiSetClassMethod(ICValuator, ICPI_CREATE,  (Imethod) valuatorcreate);
  iupCpiSetClassMethod(ICValuator, ICPI_DESTROY, (Imethod) valuatordestroy);
  iupCpiSetClassMethod(ICValuator, ICPI_SETATTR, (Imethod) valuatorsetattr);
  iupCpiSetClassMethod(ICValuator, ICPI_GETATTR, (Imethod) valuatorgetattr);
  iupCpiSetClassMethod(ICValuator, ICPI_MAP,     (Imethod) valuatormap);
}
