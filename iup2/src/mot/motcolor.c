/** \file
 * \brief Motif Driver color management
 *
 * See Copyright Notice in iup.h
 * $Id: motcolor.c,v 1.1 2008-10-17 06:19:25 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <Xm/Xm.h>
#include <X11/Xproto.h>

#include "iglobal.h"
#include "motif.h"
        
/* global variables */
        
unsigned long (* iupmot_GetPixel)(unsigned char r, unsigned char g, unsigned char b); 
void (* iupmot_GetRGB)(unsigned long pixel, unsigned char* red, unsigned char* green, unsigned char* blue);

/* local varaibles */

static int iupDirectConv[256];    /* used with directColor visuals */

/******************************************************/

static int handler(Display* dpy, XErrorEvent *err)
{
  char msg[80];

  /* Se for erro de BadAcess em XFreeColors, tudo bem */
  if (err->request_code==X_FreeColors && err->error_code==BadAccess)
    return 0;

  XGetErrorText(dpy, err->error_code, msg, 80 );
  fprintf(stderr,"IUP X11 error handler: Xlib request %d: %s\n", err->request_code, msg);

  return 0;
}

/* Busca o RGB mais proximo na tabela de cores */
static unsigned long nearest_rgb(XColor* xc1)
{
  static int nearest_try = 0;

  int pos = 0, i;
  unsigned long min_dist = ULONG_MAX, this_dist;
  int dr, dg, db;
  XColor* xc2;

  for (i=0; i<iupmot_color.num_colors; i++)
  {
    xc2 = &(iupmot_color.color_table[i]);

    dr = (xc1->red   - xc2->red) / 850;       /* 0.30 / 255 */
    dg = (xc1->green - xc2->green) / 432;     /* 0.59 / 255 */
    db = (xc1->blue  - xc2->blue) /  2318;    /* 0.11 / 255 */

    this_dist = dr*dr + dg*dg + db*db;

    if (this_dist < min_dist)
    {
      min_dist = this_dist;            
      pos = i;                          
    }
  }

  /* verifico se a cor ainda esta alocada */
  /* Try to allocate the closest match color.  This should only
     fail if the cell is read/write.  Otherwise, we're incrementing
     the cell's reference count. (comentario extraido da biblioteca Mesa) */
  if (!XAllocColor(iupmot_auxdisplay, iupmot_color.colormap,
                   &(iupmot_color.color_table[pos])))
  {
    /* nao esta, preciso atualizar a tabela e procurar novamente */
    /* isto acontece porque a cor encontrada pode ter sido de uma aplicacao que nao existe mais */
    /* uma vez atualizada, o problema nao ocorrera' na nova procura */
    /* ou a celula e' read write */

    if (nearest_try == 1)
    {
      nearest_try = 0;
      return iupmot_color.color_table[pos].pixel;
    }

    XQueryColors(iupmot_display, iupmot_color.colormap, iupmot_color.color_table, iupmot_color.num_colors);

    nearest_try = 1; /* garante que so' vai tentar isso uma vez */
    return nearest_rgb(xc1);
  }

  return iupmot_color.color_table[pos].pixel;
}

/* Funcao get_pixel usando tabela de conversao. \
   Usada quando nao estamos em TrueColor. */
static unsigned long not_truecolor_get_pixel(unsigned char cr, unsigned char cg,
                                             unsigned char cb)
{
  unsigned long pixel;
  XColor xc;
  xc.red = (unsigned short)((cr*USHRT_MAX)/UCHAR_MAX);
  xc.green = (unsigned short)((cg*USHRT_MAX)/UCHAR_MAX);
  xc.blue = (unsigned short)((cb*USHRT_MAX)/UCHAR_MAX);
  xc.flags = DoRed | DoGreen | DoBlue;

  /* verificamos se a nova cor ja' esta' disponivel */
  if (!XAllocColor(iupmot_auxdisplay, iupmot_color.colormap, &xc))
  {
    /* nao estava disponivel, procuro pela mais proxima na tabela de cores */
    pixel = nearest_rgb(&xc); 
  }
  else
  {
    /* ja' estava disponivel */
    /* atualizo a tabela de cores */
    iupmot_color.color_table[xc.pixel] = xc;
    pixel = xc.pixel;
  }

  return pixel;
}

/*
%F Funcao  usando tabela de conversao. \
   Usada quando nao estamos em TrueColor.
*/
static void not_truecolor_get_rgb(unsigned long pixel, unsigned
char* red, unsigned char* green, unsigned char* blue)
{
  XColor xc;
  xc.pixel = pixel;
  XQueryColor(iupmot_display, iupmot_color.colormap, &xc);
  *red = (unsigned char)xc.red;
  *green = (unsigned char)xc.green;
  *blue = (unsigned char)xc.blue;
}

/*
%F Funcao get_rgb usada quando estamos em TrueColor.
*/
static void truecolor_get_rgb(unsigned long pixel, unsigned
char* red, unsigned char* green, unsigned char* blue)
{
  unsigned long r = pixel & iupmot_visual->red_mask;
  unsigned long g = pixel & iupmot_visual->green_mask;
  unsigned long b = pixel & iupmot_visual->blue_mask;
  if (iupmot_color.rshift<0) r = r >> (-iupmot_color.rshift);
  else r = r << iupmot_color.rshift;
  if (iupmot_color.gshift<0) g = g >> (-iupmot_color.gshift);
  else g = g << iupmot_color.gshift;
  if (iupmot_color.bshift<0) b = b >> (-iupmot_color.bshift);
  else b = b << iupmot_color.bshift;
  *red = (unsigned char)((r*UCHAR_MAX)/USHRT_MAX);
  *green = (unsigned char)((g*UCHAR_MAX)/USHRT_MAX);
  *blue = (unsigned char)((b*UCHAR_MAX)/USHRT_MAX);
}

/*
%F Funcao get_pixel usada quando estamos em TrueColor.
*/
static unsigned long truecolor_get_pixel(unsigned char cr, unsigned char cg,
                                         unsigned char cb)
{
  unsigned long r = (cr*USHRT_MAX)/UCHAR_MAX;
  unsigned long g = (cg*USHRT_MAX)/UCHAR_MAX;
  unsigned long b = (cb*USHRT_MAX)/UCHAR_MAX;

  if (iupmot_color.rshift<0) 
    r = r << (-iupmot_color.rshift);
  else 
    r = r >> iupmot_color.rshift;

  if (iupmot_color.gshift<0) 
    g = g << (-iupmot_color.gshift);
  else 
    g = g >> iupmot_color.gshift;

  if (iupmot_color.bshift<0) 
    b = b << (-iupmot_color.bshift);
  else 
    b = b >> iupmot_color.bshift;

  r = r & iupmot_visual->red_mask;
  g = g & iupmot_visual->green_mask;
  b = b & iupmot_visual->blue_mask;

  return r | g | b;
}

static int highbit(unsigned long ul)
{
/* returns position of highest set bit in 'ul' as an integer (0-31),
  or -1 if none */
  int i;  unsigned long hb;

  hb = 0x80;  hb = hb << 24;   /* hb = 0x80000000UL */
  for (i=31; ((ul & hb) == 0) && i>=0;  i--, ul<<=1);
  return i;
}

static void makeDirectCmap(Colormap cmap)
{
  int    i, cmaplen, numgot;
  unsigned char   origgot[256];
  XColor c;
  unsigned long rmask, gmask, bmask;
  int    rshift, gshift, bshift;

  rmask = iupmot_visual->red_mask;
  gmask = iupmot_visual->green_mask;
  bmask = iupmot_visual->blue_mask;

  rshift = highbit(rmask) - 15;
  gshift = highbit(gmask) - 15;
  bshift = highbit(bmask) - 15;

  if (rshift<0) rmask = rmask << (-rshift);
  else rmask = rmask >> rshift;

  if (gshift<0) gmask = gmask << (-gshift);
  else gmask = gmask >> gshift;

  if (bshift<0) bmask = bmask << (-bshift);
  else bmask = bmask >> bshift;

  cmaplen = iupmot_visual->map_entries;
  if (cmaplen>256) cmaplen=256;

  /* try to alloc a 'cmaplen' long grayscale colormap.  May not get all
  entries for whatever reason.  Build table 'iupDirectConv[]' that
  maps range [0..(cmaplen-1)] into set of colors we did get */

  for (i=0; i<256; i++) {  origgot[i] = 0;  iupDirectConv[i] = i; }

  for (i=numgot=0; i<cmaplen; i++) 
  {
    c.red = c.green = c.blue = (i * 0xffff) / (cmaplen - 1);
    c.red   = c.red   & rmask;
    c.green = c.green & gmask;
    c.blue  = c.blue  & bmask;
    c.flags = DoRed | DoGreen | DoBlue;

    if (XAllocColor(iupmot_auxdisplay, cmap, &c)) 
    {
      origgot[i] = 1;
      numgot++;
    }
  }

  if (numgot == 0) 
    return;

  /* iupDirectConv may or may not have holes in it. */
  for (i=0; i<cmaplen; i++) 
  {
    if (!origgot[i]) 
    {
      int numbak, numfwd;
      numbak = numfwd = 0;
      while ((i - numbak) >= 0       && !origgot[i-numbak]) numbak++;
      while ((i + numfwd) <  cmaplen && !origgot[i+numfwd]) numfwd++;

      if (i-numbak<0        || !origgot[i-numbak]) numbak = 999;
      if (i+numfwd>=cmaplen || !origgot[i+numfwd]) numfwd = 999;

      if      (numbak<numfwd) iupDirectConv[i] = iupDirectConv[i-numbak];
      else if (numfwd<999)    iupDirectConv[i] = iupDirectConv[i+numfwd];
    }
  }
}

void iupmotColorInit (void)
{
  if (iupmot_depth > 8)
  {
    iupmot_GetRGB = truecolor_get_rgb;
    iupmot_GetPixel = truecolor_get_pixel;

     /* make linear colormap for DirectColor visual */
    if (iupmot_visual->class == DirectColor)
      makeDirectCmap(DefaultColormap(iupmot_display,
                     XDefaultScreen(iupmot_display)));

    iupmot_color.rshift = 15 - highbit(iupmot_visual->red_mask);
    iupmot_color.gshift = 15 - highbit(iupmot_visual->green_mask);
    iupmot_color.bshift = 15 - highbit(iupmot_visual->blue_mask);

    iupmot_color.num_colors = 0;
    iupmot_color.colormap = DefaultColormap(iupmot_display,
                                         XDefaultScreen(iupmot_display));
  }
  else
  {
    int i;
    iupmot_GetRGB = not_truecolor_get_rgb;
    iupmot_GetPixel = not_truecolor_get_pixel;

    iupmot_color.colormap = DefaultColormap(iupmot_display,
                                         XDefaultScreen(iupmot_display));
    iupmot_color.num_colors = 1L << iupmot_depth;

    for (i=0; i<iupmot_color.num_colors; i++)
      iupmot_color.color_table[i].pixel = i;

    XQueryColors(iupmot_display, iupmot_color.colormap, iupmot_color.color_table,
                 iupmot_color.num_colors);
    XSetErrorHandler(handler);
  }

}

void iupmotColorClose (void)
{
  if (iupmot_depth <= 8)
  {
    unsigned long pixels[256];
    int i;

    /* libera todas as cores usadas na palette */
    for(i = 0; i < iupmot_color.num_colors; i++)
      pixels[i] = iupmot_color.color_table[i].pixel;

    XFreeColors(iupmot_display, iupmot_color.colormap, pixels,
                iupmot_color.num_colors, 0);

    if (iupmot_color.colormap != DefaultColormap(iupmot_display,
                                              XDefaultScreen(iupmot_display)))
    {
      XFreeColormap(iupmot_display, iupmot_color.colormap);
    }
  }
}
