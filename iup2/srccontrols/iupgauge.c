/** \file
 * \brief iupgauge control
 *
 * See Copyright Notice in iup.h
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <iupcpi.h>
#include "istrutil.h"

#include <iupcompat.h>
#include "iupcdutil.h"
#include "iupgauge.h"
#include "icontrols.h"

#define GDEFAULTCOLOR "64 96 192"
#define GDEFAULTSIZE "180x20"

#define GAP 3
#define BLOCKS 20

static Ihandle *gaugecreate(Iclass *ic);
static void gaugesetattr(Ihandle *h, char *attr, char *value);
static void gaugedestroy(Ihandle *h);
static char *gaugegetattr(Ihandle *h, char *attr);

typedef struct _Tgauge
{
  int w,h;
  cdCanvas *cddbuffer;
  cdCanvas *cdcanvas;
  long bgcolor,
       fgcolor,
       light_shadow,
       mid_shadow,
       dark_shadow;
  double value,  /* value is min<value<max */
         vmin,
         vmax;
  char *text;
  int show_text,
      dashed,
      margin;
} Tgauge;

#define round(_) ((int)((_)+.5))

static void drawtext(Tgauge *g, int xmid, Ihandle* ih)
{
  int x, y, xmin, xmax, ymin, ymax;
  char* text = g->text;
  cdCanvasNativeFont(g->cddbuffer,IupGetAttribute(ih,IUP_FONT));
  cdCanvasTextAlignment(g->cddbuffer,CD_CENTER);
  cdCanvasBackOpacity(g->cddbuffer,CD_TRANSPARENT);

  x = (int)(0.5*g->w);
  y = (int)(0.5*g->h);

  if (text == NULL)
  {
    static char m[30];
    sprintf(m,"%.1f%%",100*(g->value-g->vmin)/(g->vmax-g->vmin));
    text = m;
  }

  cdCanvasGetTextBox(g->cddbuffer,x, y, text, &xmin, &xmax, &ymin, &ymax);

  if (xmid < xmin)
  {
    cdCanvasForeground(g->cddbuffer,g->fgcolor);
    cdCanvasText(g->cddbuffer,x,y,text);
  }
  else if (xmid > xmax)
  {
    cdCanvasForeground(g->cddbuffer,g->bgcolor);
    cdCanvasText(g->cddbuffer,x,y,text);
  }
  else
  {
    cdCanvasClip(g->cddbuffer,CD_CLIPAREA);
    cdCanvasClipArea(g->cddbuffer,xmin, xmid, ymin, ymax);
    cdCanvasForeground(g->cddbuffer,g->bgcolor);
    cdCanvasText(g->cddbuffer,x,y,text);

    cdCanvasClipArea(g->cddbuffer,xmid, xmax, ymin, ymax);
    cdCanvasForeground(g->cddbuffer,g->fgcolor);
    cdCanvasText(g->cddbuffer,x,y,text);
    cdCanvasClip(g->cddbuffer,CD_CLIPOFF);
  }
}

static void drawgauge(Tgauge *g, Ihandle *ih)
{
  int margem = g->margin+2; /* inclui o pixel usado para desenhar a borda 3D */
  int ystart = margem;
  int yend = g->h-1 - margem;
  int xstart = margem;
  int xend = g->w-1 - margem;

  cdIupDrawSunkenRect(g->cddbuffer, 0, 0, g->w-1, g->h-1, g->light_shadow, g->mid_shadow, g->dark_shadow);

  cdCanvasForeground(g->cddbuffer,g->fgcolor);

  if (g->dashed)
  {
    float step = (xend-xstart+1)/(float)BLOCKS;
    float boxw = step-GAP;
    float vx = (float)((xend-xstart+1) * (g->value-g->vmin)/(g->vmax-g->vmin));
    int intvx = (int)(100*vx);
    float i=0;

    if (g->value == g->vmin) return;

    while ( round(100*(i + boxw)) <= intvx )
    {
      cdCanvasBox(g->cddbuffer, xstart + round(i),
             xstart + round(i+boxw) - 1, ystart, yend );
      i += step;
    }
  }
  else
  {
    int xmid = xstart + round((xend-xstart+1) * (g->value-g->vmin)/(g->vmax-g->vmin));

    if (g->value != g->vmin)
      cdCanvasBox(g->cddbuffer, xstart, xmid, ystart, yend );

    if(g->show_text)
      drawtext(g, xmid, ih);
  }
}

static void gauge_create(Tgauge *g, Ihandle *ih)
{
  if (!g->cdcanvas)
    g->cdcanvas = cdCreateCanvas(CD_IUP, ih);

  if (g->cdcanvas)
  {
    cdCanvasActivate(g->cdcanvas);
    g->cddbuffer = cdCreateCanvas(CD_DBUFFER,g->cdcanvas);
  }
}

static int resize(Ihandle *ih)
{
  Tgauge *g=(Tgauge *)iupGetEnv(ih,"_IUPGAUGE_DATA");
  if (!g) return IUP_DEFAULT;

  if (!g->cddbuffer)
    gauge_create(g, ih);

  assert(g->cddbuffer);
  if (!g->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(g->cddbuffer);
  cdCanvasGetSize(g->cddbuffer,&g->w,&g->h,NULL,NULL);

  return IUP_DEFAULT;
}

static int repaint(Ihandle *ih)
{
  Tgauge *g=(Tgauge *)iupGetEnv(ih,"_IUPGAUGE_DATA");
  if (!g) return IUP_DEFAULT;

  if (!g->cddbuffer)
    gauge_create(g, ih);

  assert(g->cddbuffer);
  if (!g->cddbuffer)
    return IUP_DEFAULT;

  g->bgcolor = cdIupConvertColor(iupGetBgColor(ih));
  cdIupCalcShadows(g->bgcolor, &g->light_shadow, &g->mid_shadow, &g->dark_shadow);

  g->fgcolor = cdIupConvertColor(IupGetAttribute(ih, IUP_FGCOLOR));

  cdCanvasBackground(g->cddbuffer,g->bgcolor);
  cdCanvasClear(g->cddbuffer);

  drawgauge(g, ih);

  cdCanvasFlush(g->cddbuffer);

  return IUP_DEFAULT;
}

static void gaugedestroy(Ihandle *ih)
{
  Tgauge *g=(Tgauge *)iupGetEnv(ih,"_IUPGAUGE_DATA");
  if (!g) return;

  if (g->cddbuffer) cdKillCanvas(g->cddbuffer);
  if (g->cdcanvas) cdKillCanvas(g->cdcanvas);

  iupSetEnv(ih, "_IUPGAUGE_DATA", NULL);

  free(g);
}

Ihandle *IupGauge(void)
{
  return IupCreate("gauge");
}

static Ihandle *gaugecreate(Iclass *ic)
{
  Tgauge *g=(Tgauge*)malloc(sizeof(Tgauge));
  Ihandle *self = IupCanvas(NULL);
  (void)ic;

  iupSetEnv(self,"_IUPGAUGE_DATA",(char*)g); /* hangs valuator data in canvas */

  IupSetCallback(self,IUP_RESIZE_CB,(Icallback)resize);
  IupSetCallback(self,IUP_ACTION,(Icallback)repaint);

  iupSetEnv(self,IUP_EXPAND, IUP_NO);
  iupSetEnv(self,IUP_BORDER, IUP_NO);
  iupSetEnv(self,IUP_SCROLLBAR, IUP_NO);
  iupSetEnv(self,IUP_FGCOLOR, GDEFAULTCOLOR);
  iupSetEnv(self,IUP_SIZE, GDEFAULTSIZE);

  memset(g, 0, sizeof(Tgauge));

  g->fgcolor = cdIupConvertColor(GDEFAULTCOLOR);

  g->vmax = 1;
  g->bgcolor = CD_GRAY;
  g->light_shadow = CD_WHITE;
  g->mid_shadow = CD_GRAY;
  g->dark_shadow = CD_DARK_GRAY;

  g->show_text = 1;
  g->dashed    = 0;
  g->margin    = 1;

  return self;
}

static void crop_value(Tgauge *g)
{
  if (g->value>g->vmax) g->value=g->vmax;
  else if (g->value<g->vmin) g->value=g->vmin;
}

static void gaugesetattr(Ihandle *h, char *attr, char *value)
{
  Tgauge *g=(Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");

  if (!g)
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  if(iupStrEqual(attr,IUP_VALUE))  /* faz update no caso de setar o valor */
  {
    if(value == NULL)
      g->value = 0;
    else
      g->value = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr, IUP_MIN)) 
  {
    if(value==NULL) /* reseta para default */
      g->vmin = 0;
    else
      g->vmin = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr, IUP_MAX))
  {
    if(value==NULL) /* reseta para default */
      g->vmax = 1;
    else
      g->vmax = atof(value);
    crop_value(g);
  }
  else if(iupStrEqual(attr,ICTL_SHOW_TEXT))
  {
    if(iupStrEqual(value, IUP_YES))
      g->show_text = 1;
    else if(iupStrEqual(value, IUP_NO))
      g->show_text = 0;
  }
  else if(iupStrEqual(attr,ICTL_MARGIN))
  {
    if ( value == NULL ) g->margin = 1;
    else g->margin = atoi( value );
  }
  else if(iupStrEqual(attr,ICTL_DASHED))
  {
    if(iupStrEqual(value, IUP_YES))
      g->dashed = 1;
    else if(iupStrEqual(value, IUP_NO))
      g->dashed = 0;
  }
  else if(iupStrEqual(attr,ICTL_TEXT))
  {
    char *tmp ;

    if (value == NULL)
    {
      if(g->text != NULL)
        free(g->text);
      g->text = NULL;
    }
    else
    {
      tmp = (char*)iupStrDup(value);

      if(g->text != NULL)
        free(g->text);

      if(tmp != NULL)
        g->text = tmp;
    }
  }
  else
  {
    iupCpiDefaultSetAttr(h, attr, value); /* chama metodo default */
    return;
  }

  /* redesenha controle caso ele ja' tenha sido mapeado */
  if (g->cddbuffer)  
    repaint(h);
}

static char *gaugegetattr(Ihandle *h, char *attr)
{
  Tgauge *g = (Tgauge *)iupGetEnv(h,"_IUPGAUGE_DATA");

  if(g == NULL)
    return iupCpiDefaultGetAttr(h, attr); /* chama metodo default */

  if(iupStrEqual(attr, IUP_VALUE))
  {
    static char value[30];
    sprintf(value, "%g", g->value);
    return value;
  }
  else if(iupStrEqual(attr, ICTL_TEXT))
    return g->text;
  else if(iupStrEqual(attr, ICTL_SHOW_TEXT))
  {
    if(g->show_text)
      return IUP_YES;
    else
      return IUP_NO;
  }
  else if(iupStrEqual(attr, ICTL_MARGIN))
  { 
    static char buffer[ 10 ];
    sprintf( buffer, "%d", g->margin );
    return buffer;
  }
  else if(iupStrEqual(attr, ICTL_DASHED))
  {
    if(g->dashed)
      return IUP_YES;
    else
      return IUP_NO;
  }
  else
    return iupCpiDefaultGetAttr(h, attr); /* chama metodo default */
}

void IupGaugeOpen(void)
{
  Iclass* ICGauge = iupCpiCreateNewClass("gauge", NULL);

  iupCpiSetClassMethod(ICGauge, ICPI_CREATE, (Imethod) gaugecreate);
  iupCpiSetClassMethod(ICGauge, ICPI_DESTROY, (Imethod) gaugedestroy);
  iupCpiSetClassMethod(ICGauge, ICPI_SETATTR, (Imethod) gaugesetattr);
  iupCpiSetClassMethod(ICGauge, ICPI_GETATTR, (Imethod) gaugegetattr);
}
