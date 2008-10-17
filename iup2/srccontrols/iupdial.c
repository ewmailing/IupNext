/** \file
* \brief iupdial control
*
* See Copyright Notice in iup.h
* $Id: iupdial.c,v 1.1 2008-10-17 06:19:56 scuri Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "iup.h"
#include "iupcpi.h"

#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include "iupcompat.h"
#include "iupdial.h"
#include "iupcdutil.h"
#include "icontrols.h"

#define SPACE 3
#define NCOLORS 10
#define pi	3.1416
#define IDIAL_DEFAULT_DENSITY .2 /* Numero default de pixels por  "traco" no dial */

#define IDIAL_VERTICAL    1
#define IDIAL_HORIZONTAL  2
#define IDIAL_CIRCULAR    3

#define valmin(a,b)	((a)<(b)?(a):(b))

typedef int (*Aplcb)(Ihandle *,double);

typedef struct _Tdial
{
  int w,h;
  cdCanvas *cddbuffer;
  cdCanvas *cdcanvas;
  long fgcolor[NCOLORS+1];
  void (*drawdial)(struct _Tdial *);
  double alpha;
  int px,
      py,
      pressing,
      type,
      has_focus, 
      active,
      num_div;
  double unit,
         radius,
         density;
  long bgcolor,
       light_shadow,
       mid_shadow,
       dark_shadow;
  unsigned char fgred,
                fggreen,
                fgblue;
} Tdial;

static void beginimage(Tdial *d, Ihandle* self)
{
  d->bgcolor = cdIupConvertColor(iupGetBgColor(self));
  cdIupCalcShadows(d->bgcolor, &d->light_shadow, &d->mid_shadow, &d->dark_shadow);

  if (!d->active) 
    d->light_shadow = d->mid_shadow;
  
  cdCanvasBackground(d->cddbuffer,d->bgcolor);
  cdCanvasClear(d->cddbuffer);
}

static void putimage(Tdial *d, Ihandle* self)
{
  cdCanvasFlush(d->cddbuffer);

  if (d->has_focus)
    cdIupDrawFocusRect(self, d->cdcanvas, 0, 0, d->w-1, d->h-1);
}

static int resize_cb(Ihandle *self);

static void drawdial(Tdial *d, Ihandle* self)
{
  if(d->cdcanvas == NULL || d->cddbuffer == NULL)
    resize_cb(self);

  beginimage(d,self);
  d->drawdial(d);
  putimage(d, self);
}

static long int getcolor(Tdial *d,double a,double amin)
{
  double nu=fabs(a-0.5*pi);
  double de=fabs(0.5*pi-amin);
  double fr=nu/de;
  int i=(int)(NCOLORS*fr);
  return d->fgcolor[NCOLORS-i];
}

static void drawvdialback(Tdial *d, double amin, double amax, int *ymin, int *ymax)
{
  double delta=(0.5*pi-amin)/NCOLORS;
  double a, yc = d->h/2.0;
  *ymin = *ymax = d->h/2;
  for (a=amin;a<0.5*pi;a+=delta)		/* shading */
  {
    int y0=(int)(yc - d->radius*cos(a));
    int y1=(int)(yc - d->radius*cos(a+delta));
    cdCanvasForeground(d->cddbuffer,getcolor(d,a,amin));
    cdCanvasBox(d->cddbuffer,SPACE+1, d->w-1-SPACE-2,y0,y1);

    if (y0 < *ymin) *ymin = y0;

    if (abs(y1-y0) < 2)
      continue;
  }
  for (a=0.5*pi;a<amax;a+=delta)
  {
    int y0=(int)(yc + d->radius*fabs(cos(a)));
    int y1=(int)(yc + d->radius*fabs(cos(a+delta)));
    cdCanvasForeground(d->cddbuffer,getcolor(d,a,amin));
    cdCanvasBox(d->cddbuffer,SPACE+1, d->w-1-SPACE-2,y0,y1);

    if (y1 > *ymax) *ymax = y1;

    if (abs(y1-y0) < 2)
      continue;
  }
}

static void drawVdial(Tdial *d)
{
  double delta=2*pi/d->num_div;
  double a, amin, amax;
  int ymin, ymax;

  d->radius = (d->h-2*SPACE-2)/2.0;

  amin=0.0;
  amax=pi;
  if(d->alpha<amin)
  {
    for (a=d->alpha;a<amin;a+=delta)
      ;
  }
  else
  {
    for (a=d->alpha;a>amin;a-=delta)
      ;
    a+=delta;
  }

  drawvdialback(d,amin,amax, &ymin, &ymax);

  cdIupDrawRaisenRect(d->cddbuffer, SPACE, ymin, d->w-1-SPACE, ymax,
                      d->light_shadow, d->mid_shadow, d->dark_shadow);

  for (;a<amax;a+=delta)		/* graduation */
  {
    int y;
    if (a<0.5*pi) y=(int)(d->h/2.0 - d->radius*cos(a));
    else          y=(int)(d->h/2.0 + d->radius*fabs(cos(a)));

    if (abs(y-ymin) < 3 || abs(ymax-y) < 3)
      continue;

    cdIupDrawHorizSunkenMark(d->cddbuffer, SPACE+1, d->w-1-SPACE-2, y, d->light_shadow, d->dark_shadow);
  }
}

static void drawhdialback(Tdial *d,double amin,double amax, int *xmin, int *xmax)
{
  double delta=(0.5*pi-amin)/NCOLORS;
  double a, xc = d->w/2.0;
  *xmin = *xmax = d->w/2;
  for (a=amin;a<0.5*pi;a+=delta)
  {
    int x0=(int)(xc - d->radius*cos(a));
    int x1=(int)(xc - d->radius*cos(a+delta));
    cdCanvasForeground(d->cddbuffer,getcolor(d,a,amin));
    cdCanvasBox(d->cddbuffer,x0,x1,SPACE+2,d->h-1-SPACE-1);

    if (x0 < *xmin) *xmin = x0;

    if (abs(x1-x0) < 2)
      continue;
  }
  for (a=0.5*pi;a<amax;a+=delta)
  {
    int x0=(int)(xc + d->radius*fabs(cos(a)));
    int x1=(int)(xc + d->radius*fabs(cos(a+delta)));
    cdCanvasForeground(d->cddbuffer,getcolor(d,a,amin));
    cdCanvasBox(d->cddbuffer,x0,x1,SPACE+2,d->h-1-SPACE-1);

    if (x1 > *xmax) *xmax = x1;

    if (abs(x1-x0) < 2)
      continue;
  }
}

static void drawHdial(Tdial *d)
{
  double delta=2*pi/d->num_div;
  int x;
  double a,amin,amax;
  int xmin, xmax;
  d->radius = (d->w-2*SPACE-2)/2.0;
  amin=0.0;
  amax=pi;
  if(d->alpha<amin)
  {
    for (a=d->alpha;a<amin;a+=delta) ;
  }
  else
  {
    for (a=d->alpha;a>amin;a-=delta) ;
    a+=delta;
  }

  drawhdialback(d,amin,amax, &xmin, &xmax);

  cdIupDrawRaisenRect(d->cddbuffer, xmin, SPACE, xmax, d->h-1-SPACE,
                      d->light_shadow, d->mid_shadow, d->dark_shadow);

  for (;a<amax;a+=delta)
  {
    if (a<0.5*pi) x=(int)(d->w/2.0 - d->radius*cos(a));
    else          x=(int)(d->w/2.0 + d->radius*fabs(cos(a)));

    if (abs(x-xmin) < 3 || abs(xmax-x) < 3)
      continue;

    cdIupDrawVertSunkenMark(d->cddbuffer, x, SPACE+2, d->h-1-SPACE-1, d->light_shadow, d->dark_shadow);
  }
}

static void drawbol(Tdial *d, int x1, int y1)
{
  cdCanvasForeground(d->cddbuffer,d->bgcolor);
  cdCanvasBox(d->cddbuffer,x1, x1+4, y1, y1+4);

  cdCanvasForeground(d->cddbuffer,d->light_shadow);
  cdCanvasLine(d->cddbuffer,x1,y1+1,x1,y1+3);
  cdCanvasLine(d->cddbuffer,x1+1,y1+4,x1+3,y1+4);

  cdCanvasForeground(d->cddbuffer,d->mid_shadow);
  cdCanvasLine(d->cddbuffer,x1+1,y1,x1+4,y1);
  cdCanvasLine(d->cddbuffer,x1+4,y1,x1+4,y1+3);

  cdCanvasForeground(d->cddbuffer,d->dark_shadow);
  cdCanvasLine(d->cddbuffer,x1+2,y1,x1+3,y1);
  cdCanvasLine(d->cddbuffer,x1+4,y1+1,x1+4,y1+2);
}

static void drawCdial(Tdial *d)
{
  double delta=2*pi/d->num_div, a=d->alpha;
  int i, xc=d->w/2, yc=d->h/2, wide;
  d->radius=valmin(d->w,d->h)/2-2*SPACE;

  wide = (int)(2*d->radius);
  cdCanvasForeground(d->cddbuffer,d->mid_shadow);
  cdCanvasLineWidth(d->cddbuffer,2);
  cdCanvasArc(d->cddbuffer,xc,yc,wide-1,wide-1,-135,45.0);
  cdCanvasLineWidth(d->cddbuffer,1);
  cdCanvasForeground(d->cddbuffer,d->bgcolor);
  cdCanvasSector(d->cddbuffer,xc,yc,wide-2,wide-2,0.0,360.0);
  cdCanvasForeground(d->cddbuffer,d->light_shadow);
  cdCanvasArc(d->cddbuffer,xc,yc,wide,wide,45,225);
  cdCanvasForeground(d->cddbuffer,d->dark_shadow);
  cdCanvasArc(d->cddbuffer,xc,yc,wide,wide,-135,45);

  for (i=0;i<d->num_div;++i)
  {
    int x2=(int)(xc + (d->radius - 6)*cos(a));
    int y2=(int)(yc + (d->radius - 6)*sin(a));

    if (i == 0)
    {
      cdCanvasForeground(d->cddbuffer,CD_BLACK);
      cdCanvasLine(d->cddbuffer,xc, yc, x2, y2);
    }

    drawbol(d, x2-2, y2-2);
    a+=delta;
  }

  drawbol(d, xc-2, yc-2);
}

static int motionV_cb(Ihandle *self,int x,int y,char *r)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb aplmmv;

  if (!isbutton1(r)) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  y = cdIupInvertYAxis(y, d->h);
  d->alpha+=(double)(y-d->py)/d->radius;
  d->py=y;

  drawdial(d, self);

  aplmmv = (Aplcb) IupGetCallback(self, ICTL_MOUSEMOVE_CB);
  if (aplmmv!=NULL)
    aplmmv(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int motionH_cb(Ihandle *self,int x,int y,char *r)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb aplmmv;

  if (!isbutton1(r)) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  y = cdIupInvertYAxis(y, d->h);
  d->alpha+=(double)(x-d->px)/d->radius;
  d->px=x;

  drawdial(d, self);

  aplmmv = (Aplcb) IupGetCallback(self, ICTL_MOUSEMOVE_CB);
  if (aplmmv!=NULL)
    aplmmv(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int motionC_cb(Ihandle *self,int x,int y,char *r)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb aplmmv;

  int cx=d->w/2;
  int cy=d->h/2;
  double vet,xa,ya,xb,yb,ma,mb,ab;

  if (!isbutton1(r)) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  y = cdIupInvertYAxis(y, d->h);

  xa=d->px-cx; 
  ya=d->py-cy; 
  ma=sqrt(xa*xa + ya*ya);

  xb=x-cx;  
  yb=y-cy;  
  mb=sqrt(xb*xb + yb*yb);

  ab=xa*xb+ya*yb;
  vet=xa*yb-xb*ya;

  ab = ab/(ma*mb);

  /* se mouse estiver no centro do dial, ignora */
  if(ma == 0 || mb == 0 || ab < -1 || ab > 1)
    return IUP_DEFAULT;

  if (vet>0) d->alpha += acos(ab);
  else       d->alpha -= acos(ab);

  drawdial(d, self);
  d->px=x; d->py=y;

  aplmmv = (Aplcb) IupGetCallback(self, ICTL_MOUSEMOVE_CB);
  if (aplmmv!=NULL)
    aplmmv(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int btp(Ihandle *self,int v,int x,int y)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb aplbtp;

  if (v!=IUP_BUTTON1) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  y = cdIupInvertYAxis(y, d->h);
  d->px=x; d->py=y;

  if (d->type != IDIAL_CIRCULAR)
    d->alpha=0;

  aplbtp = (Aplcb) IupGetCallback(self, ICTL_BUTTON_PRESS_CB);

  if (aplbtp!=NULL)
    aplbtp(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int btr(Ihandle *self,int v)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb aplbtr;

  if (v!=IUP_BUTTON1) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  drawdial(d, self);

  aplbtr = (Aplcb) IupGetCallback(self, ICTL_BUTTON_RELEASE_CB);
  if (aplbtr!=NULL)
    aplbtr(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int button_cb(Ihandle *self,int b,int m,int x,int y)
{
  if (m) return btp(self,b,x,y);
  else   return btr(self,b);
}

static void setscale(Tdial *d)
{
  int i;
  unsigned char r=d->fgred;
  unsigned char g=d->fggreen;
  unsigned char b=d->fgblue;
  int max = d->active? 255: 192;
  int deltar=(max-r)/NCOLORS;
  int deltag=(max-g)/NCOLORS;
  int deltab=(max-b)/NCOLORS;
  for (i=0;i<=NCOLORS;++i)
  {
    d->fgcolor[i]=cdEncodeColor(r,g,b);
    r += deltar;
    g += deltag;
    b += deltab;
  }
}

static void createcanvas(Tdial *d, Ihandle *self)
{
  if (!d->cdcanvas)
    d->cdcanvas = cdCreateCanvas(CD_IUP, self);

  if (d->cdcanvas)
  {
    cdCanvasActivate(d->cdcanvas);
    d->cddbuffer = cdCreateCanvas(CD_DBUFFER, d->cdcanvas);
  }
}

static int resize_cb(Ihandle *self)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  if (!d) return IUP_DEFAULT;

  if (!d->cddbuffer)
    createcanvas(d, self);

  if (!d->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(d->cddbuffer);
  cdCanvasGetSize(d->cddbuffer,&d->w,&d->h,NULL,NULL);

  switch(d->type)
  {
  case IDIAL_VERTICAL:
    d->num_div = (int)((d->h-2*SPACE-2) * d->density);
    break;

  case IDIAL_HORIZONTAL:
    d->num_div = (int)((d->w-2*SPACE-2) * d->density);
    break;

  case IDIAL_CIRCULAR:
    d->num_div = (int)((valmin(d->w,d->h)-2*SPACE-2)*d->density);
    break;
  }
  
  return IUP_DEFAULT;
}

static int repaint_cb(Ihandle *self)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  if (!d) return IUP_DEFAULT;

  if (!d->cddbuffer)
    createcanvas(d, self);

  if (!d->cddbuffer)
    return IUP_DEFAULT;

  drawdial(d, self);

  return IUP_DEFAULT;
}

static int focus_cb (Ihandle * self, int focus)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  if (d == NULL) return IUP_DEFAULT;
  if (!d->active) return IUP_IGNORE;

  d->has_focus = focus;
  drawdial(d, self);

  return IUP_DEFAULT;
}

static int keypress_cb(Ihandle *self, int c, int press)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  char* cb_name;
  Aplcb cb;

  if (d == NULL) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  if (c != K_LEFT && c != K_UP &&
      c != K_sLEFT && c != K_sUP &&
      c != K_RIGHT && c != K_DOWN &&
      c != K_sRIGHT && c != K_sDOWN)
    return IUP_DEFAULT;

  if (d->pressing)
  {
    switch(c)
    {
    case K_sLEFT:
    case K_sDOWN:
    case K_LEFT:
    case K_DOWN:
      d->alpha -= pi/10.0;
      break;
    case K_sRIGHT:
    case K_sUP:
    case K_RIGHT:
    case K_UP:
      d->alpha += pi/10.0;
      break;
    }

    if (fabs(d->alpha) < pi/10.1)
      d->alpha = 0;
  }

  if (press)
  {
    if (d->pressing)
    {
      cb_name = ICTL_MOUSEMOVE_CB;
    }
    else
    {
      d->pressing = 1;
      d->alpha = 0;
      cb_name = ICTL_BUTTON_PRESS_CB;
    }
  }
  else
  {
    d->pressing = 0;
    cb_name = ICTL_BUTTON_RELEASE_CB;
  }

  drawdial(d, self);

  cb = (Aplcb) IupGetCallback(self, cb_name);
  if (cb) cb(self,d->alpha*d->unit);

  return IUP_DEFAULT;
}

static int wheel_cb(Ihandle *self, float delta)
{
  Tdial *d=(Tdial *)iupGetEnv(self,"_IUPDIAL_DATA");
  Aplcb cb;

  if (d == NULL) return IUP_DEFAULT;
  if (!d->active) return IUP_DEFAULT;

  d->alpha += ((double)delta)*(pi/10.0);

  if (fabs(d->alpha) < pi/10.1)
    d->alpha = 0;

  drawdial(d, self);

  cb = (Aplcb) IupGetCallback(self, ICTL_MOUSEMOVE_CB);
  if (cb) cb(self, d->alpha*d->unit);

  return IUP_DEFAULT;
}

static void dialdestroy(Ihandle *h)
{
  Tdial *d=(Tdial *)iupGetEnv(h,"_IUPDIAL_DATA");
  if (d==NULL) return;

  if (d->cddbuffer) cdKillCanvas(d->cddbuffer);
  if (d->cdcanvas) cdKillCanvas(d->cdcanvas);

  iupSetEnv(h, "_IUPDIAL_DATA", NULL);

  free(d);
}

static char* dialgetattr(Ihandle *h, char *attr)
{
  static char str[20];

  if(iupStrEqual(attr,IUP_VALUE))
  {
    Tdial *d=(Tdial *)iupGetEnv(h,"_IUPDIAL_DATA");
    if (d) 
    {
      sprintf( str, "%f", d->alpha );
      return str;
    }
  }

  return iupCpiDefaultGetAttr(h, attr);
}

static void dialsetattr(Ihandle *h, char *attr, char *value)
{
  Tdial *d=(Tdial *)iupGetEnv(h,"_IUPDIAL_DATA");

  if (d==NULL) 
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  if(iupStrEqual(attr,IUP_VALUE))
  {
    if(value==NULL) /* reseta para default */
      d->alpha = 0;
    else
      d->alpha = atof(value);
  }
  else if(iupStrEqual(attr,ICTL_DENSITY))
  {
    if(value)
      d->density = atof(value);
    else
      d->density = IDIAL_DEFAULT_DENSITY;
  }
  else if(iupStrEqual(attr,IUP_ACTIVE))
  {
    if(value==NULL) /* reseta para default */
      d->active = 1;
    else
      d->active = iupStrEqual(value,IUP_YES)? 1: 0;
    setscale(d);
  }
  else if(iupStrEqual(attr, IUP_FGCOLOR))
  {
    unsigned int r, g, b;
    iupGetRGB(value, &r, &g, &b);
    d->fgred = (unsigned char)r;
    d->fggreen = (unsigned char)g;
    d->fgblue = (unsigned char)b;
    setscale(d);
  }
  else if(iupStrEqual(attr, ICTL_UNIT))
  {
    if(value==NULL) /* reseta para default */
      d->unit = 1;
    else
      d->unit = iupStrEqual(value,"DEGREES")? CD_RAD2DEG: 1;
  }
  else if(iupStrEqual(attr, "TYPE"))
  {
    if(iupStrEqual(value, ICTL_VERTICAL))
    {
      IupSetCallback(h,IUP_MOTION_CB,(Icallback)motionV_cb);
      iupSetEnv(h, IUP_SIZE, "16x80");
      iupSetEnv (h, "TYPE", ICTL_VERTICAL);
      d->drawdial=drawVdial;
      d->type = IDIAL_VERTICAL;
    }
    else if(iupStrEqual(value, ICTL_CIRCULAR))
    {
      IupSetCallback(h,IUP_MOTION_CB,(Icallback)motionC_cb);
      iupSetEnv(h, IUP_SIZE, "40x35");
      iupSetEnv (h, "TYPE", ICTL_CIRCULAR);
      d->drawdial=drawCdial;
      d->type = IDIAL_CIRCULAR;
    }
    else 
    {
      IupSetCallback(h,IUP_MOTION_CB,(Icallback)motionH_cb);
      iupSetEnv(h, IUP_SIZE, "80x16");
      iupSetEnv (h, "TYPE", ICTL_HORIZONTAL);
      d->drawdial=drawHdial;
      d->type = IDIAL_HORIZONTAL;
    }
  }

  iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */

  if(IupGetAttribute(h, IUP_WID) != NULL && d->cddbuffer != NULL)
    drawdial(d, h);
}

static Ihandle *dialcreate(Iclass *ic, Ihandle **params)
{
  Tdial *d;
  Ihandle *self = IupCanvas(NULL);
  char *type = params? (char*) params[0] : NULL;
  (void)ic;

  d = (Tdial*) malloc(sizeof(Tdial)); 
  memset(d, 0, sizeof(Tdial)); 

  d->density=IDIAL_DEFAULT_DENSITY; 
  d->fgred=64;
  d->fggreen=64;
  d->fgblue=64;
  d->active = 1;
  d->unit = 1;

  setscale(d);

  iupSetEnv(self,"_IUPDIAL_DATA",(char*) d);	
  iupSetEnv(self,IUP_EXPAND, IUP_NO);
  iupSetEnv(self,IUP_BORDER, IUP_NO);
  iupSetEnv(self,IUP_SCROLLBAR, IUP_NO);
  iupSetEnv(self,ICTL_UNIT, "RADIANS");
  iupSetEnv(self,IUP_FGCOLOR, "64 64 64");
  iupSetEnv(self,ICTL_DENSITY, "0.2");

  IupSetCallback(self,"RESIZE_CB",(Icallback)resize_cb);
  IupSetCallback(self,"ACTION",(Icallback)repaint_cb);
  IupSetCallback(self,"BUTTON_CB",(Icallback)button_cb);
  IupSetCallback(self,"FOCUS_CB",(Icallback)focus_cb);
  IupSetCallback(self,"WHEEL_CB",(Icallback)wheel_cb);
  IupSetCallback(self,"KEYPRESS_CB",(Icallback)keypress_cb);

  if(type)
    dialsetattr(self, "TYPE", type);

  return self;
}

Ihandle *IupDial(const char *type)
{
  return IupCreatep("dial", (void*) type, NULL);
}

void IupDialOpen(void)
{
  Iclass *ICDial = iupCpiCreateNewClass("dial","s");

  iupCpiSetClassMethod(ICDial, ICPI_CREATE, (Imethod) dialcreate);
  iupCpiSetClassMethod(ICDial, ICPI_DESTROY, (Imethod) dialdestroy);
  iupCpiSetClassMethod(ICDial, ICPI_SETATTR, (Imethod) dialsetattr);
  iupCpiSetClassMethod(ICDial, ICPI_GETATTR, (Imethod) dialgetattr);
}
