/** \file
 * \brief iuptabs control
 *
 * See Copyright Notice in iup.h
 *  */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <iup.h>
#include <iupkey.h>
#include <iupcpi.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>

#include <iupcompat.h>
#include "iupcdutil.h"
#include "iuptabs.h"
#include "icontrols.h"
#include "istrutil.h"


/*
 *  Constantes
 */

static const int TABS_BORDER = 1;
static const int TABS_MARGIN = 10;
static const int TABS_SPACING = 2;
static const int TABS_SCROLL_LENGTH = 51; /* the length of the scroll control in any direction */
static const int TABS_SCROLL_THICK = 17;
static const int TABS_SCROLL_SPACING = 7;
static const int TABS_BROKEN_TAB = 8;
static const int TABS_CURRENT_EXTRA_PIXELS = 1;

/* enum's */

typedef enum
{
  TABS_TOP, TABS_BOTTOM, TABS_LEFT, TABS_RIGHT
} tTabsType;

typedef enum
{
  TABS_FALSE, TABS_TRUE
} tBool;

typedef enum
{
  BROKEN_NONE,
  BROKEN_START,
  BROKEN_END,
  BROKEN_CENTER
} tBrokenTabType;

typedef enum
{
  TABS_HORIZONTAL, TABS_VERTICAL
} tTextOrientation;

typedef enum
{
  BUT_NONE, 
  BUTPRESS_FORWARD, BUTPRESS_MENU, BUTPRESS_BACKWARD,   /* can not change order because of "+= 3" */
  BUTRELEASE_FORWARD, BUTRELEASE_MENU, BUTRELEASE_BACKWARD  
} tButtonState;

typedef enum
{
  RENDER_PLANE_ALIGN, RENDER_PLANE_NORMAL
} tTextRender;

/* informacoes sobre como desenhar cada tab */

typedef struct _tagTabDrawInfo
{
  Ihandle *ihandle;    /* child handle */
  long text_w,         /* width of the text of this Tab */
       tab_len;        /* len */
} tTabDrawInfo;

typedef struct _tagTabsLine
{
  /* quais tabs sao visiveis */
  int start_tab, end_tab;

  /* indica se algum botao esta pressionado */
  tButtonState button;

  /* informacoes sobre como desenhar cada tab */
  tTabDrawInfo* tabs_info;

  /* indica se todos os tabs sao visiveis */
  tBool scrollbar_visible, 
        broken_start_visible, 
        broken_end_visible;

  /* indica se apenas o current tab e' visivel, havendo outros tabs a
   * direita ou a esquerda */
  tBool broken_center_visible;

  /* espaco disponivel para as broken_tabs */
  long broken_tab_space,
       broken_space_start,
       broken_space_end;

  /* tabs line size */
  long line_thick,
       line_length;
  long *line_w,  /* point to thick and length according to render */
       *line_h;

  /* tabs scrollbar size */
  long scroll_len,  /* the scroll space in the tabs line length direction */
       scroll_thick,
       scroll_x,
       scroll_y,
       scroll_w,
       scroll_h;

  long text_h; /* height of the text (all texts have the same height) */
} tTabsLine;

/* contexto do controle */

typedef struct _tagTabs
{
  /* tipo de tab */
  tTabsType tabs_type;
  tTextOrientation tabs_orientation;
  tTextRender tabs_render; /* tabs line render (depends on tabs type and orientation) */

  /* handles */
  cdCanvas *cdcanvas, 
           *cddbuffer;
  Ihandle *zbox; /* zbox is a child of Tabs */
  Ihandle *self;

  /* numero de tabs */
  int number_of_tabs;

  /* tab selecionado */
  int current_tab;

  /* dados de uma fila de tabs */
  tTabsLine tabs_line;

  /* dados de largura e posicao */
  long client_w, client_h;
  long client_x, client_y;
  long w, h;
  long *max_line_length; /* pointer to w or h */

  /* cores utilizadas */
  long bgcolor;
  long fgcolor;
  long light_shadow;
  long mid_shadow;
  long dark_shadow;

  char *fontact;
  char *fontinact;
} tTabs;


static void doRepaint(tTabs *tabs);


static void updateRender(tTabs* tabs)
{
  if (tabs->tabs_orientation == TABS_VERTICAL)
  {
    if (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)
      tabs->tabs_render = RENDER_PLANE_ALIGN;
    else
      tabs->tabs_render = RENDER_PLANE_NORMAL;
  }
  else
  {
    if (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)
      tabs->tabs_render = RENDER_PLANE_NORMAL;
    else
      tabs->tabs_render = RENDER_PLANE_ALIGN;
  }

  if (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)
  {
    tabs->tabs_line.line_w = &tabs->tabs_line.line_thick;
    tabs->tabs_line.line_h = &tabs->tabs_line.line_length;
    tabs->max_line_length = &tabs->h;
  }
  else
  {
    tabs->tabs_line.line_w = &tabs->tabs_line.line_length;
    tabs->tabs_line.line_h = &tabs->tabs_line.line_thick;
    tabs->max_line_length = &tabs->w;
  }

  if (tabs->tabs_render == RENDER_PLANE_NORMAL)
  {
    tabs->tabs_line.scroll_len = TABS_SCROLL_THICK;
    tabs->tabs_line.scroll_thick = TABS_SCROLL_LENGTH;
  }
  else
  {
    tabs->tabs_line.scroll_len = TABS_SCROLL_LENGTH;
    tabs->tabs_line.scroll_thick = TABS_SCROLL_THICK;
  }

  if (tabs->tabs_orientation == TABS_HORIZONTAL)
  {
    tabs->tabs_line.scroll_w = TABS_SCROLL_LENGTH;
    tabs->tabs_line.scroll_h = TABS_SCROLL_THICK;
  }
  else
  {
    tabs->tabs_line.scroll_w = TABS_SCROLL_THICK;
    tabs->tabs_line.scroll_h = TABS_SCROLL_LENGTH;
  }
}

static void setcdfont(tTabs* tabs, char* tab_font, int bold)
{
  if (tab_font) 
    cdCanvasNativeFont(tabs->cddbuffer, tab_font);
  else 
    cdCanvasNativeFont(tabs->cddbuffer, IupGetAttribute(tabs->self, IUP_FONT)); 

  if (bold) 
    cdCanvasFont(tabs->cddbuffer, NULL, CD_BOLD, 0);
}

/*
* Retorna nome associado a um tab
*/

static Ihandle *GetTabIhandle (tTabs * tabs, int number)
{
  return tabs->tabs_line.tabs_info[number].ihandle;
}

/**********************
* Funcoes de desenho *
**********************/

/*
* Preenche area
*/

static void FillArea (cdCanvas *canvas, int x1, int y1, int x2, int y2,
                      int x3, int y3, int x4, int y4)
{
  cdCanvasBegin (canvas, CD_FILL);
  cdCanvasVertex (canvas, x1, y1);
  cdCanvasVertex (canvas, x2, y2);
  cdCanvasVertex (canvas, x3, y3);
  cdCanvasVertex (canvas, x4, y4);
  cdCanvasEnd (canvas);
  cdCanvasBegin (canvas, CD_CLOSED_LINES);
  cdCanvasVertex (canvas, x1, y1);
  cdCanvasVertex (canvas, x2, y2);
  cdCanvasVertex (canvas, x3, y3);
  cdCanvasVertex (canvas, x4, y4);
  cdCanvasEnd (canvas);
}

/*
* Desenha bordas quebradas
*/

static void DrawBrokenBorder (long x1, long y1, long x2, long y2,
                              tTabsType type, tTabs * tabs)
{
  cdCanvas *canvas = tabs->cddbuffer;
  double step_x = (x2 - x1) / 5;
  double step_y = (y2 - y1) / 5;
  long old_color;

  old_color = cdCanvasForeground (canvas, CD_QUERY);

  switch (type)
  {
  case TABS_TOP:
    cdCanvasForeground (canvas, tabs->mid_shadow);
    cdCanvasLine (canvas, (long)(x1 + step_x * 3.), y1, (long)(x1 + 4. * step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x), y1, (long)(x1 + 2. * step_x), y2);

    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, (long)(x1 + step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x), y1 - 1, (long)(x1 + 2. * step_x), y2 - 1);
    cdCanvasLine (canvas, (long)(x1 + step_x * 2.), y1, (long)(x1 + 3. * step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x * 3.), y1 + 1, (long)(x1 + 4. * step_x), y2 + 1);
    cdCanvasLine (canvas, (long)(x1 + step_x * 4.), y1, x2, y2);
    break;

  case TABS_BOTTOM:
    cdCanvasForeground (canvas, tabs->mid_shadow);
    cdCanvasLine (canvas, (long)(x1 + step_x * 3.), y1, (long)(x1 + 4. * step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x), y1, (long)(x1 + 2. * step_x), y2);

    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, (long)(x1 + step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x), y1 - 1, (long)(x1 + 2. * step_x), y2 - 1);
    cdCanvasLine (canvas, (long)(x1 + step_x * 2.), y1, (long)(x1 + 3. * step_x), y2);
    cdCanvasLine (canvas, (long)(x1 + step_x * 3.), y1 + 1, (long)(x1 + 4. * step_x), y2 + 1);
    cdCanvasLine (canvas, (long)(x1 + step_x * 4.), y1, x2, y2);
    break;

  case TABS_LEFT:
    cdCanvasForeground (canvas, tabs->mid_shadow);
    cdCanvasLine (canvas, x1, (long)(y1 + 3. * step_y), x2, (long)(y1 + step_y * 4.));
    cdCanvasLine (canvas, x1, (long)(y1 + step_y), x2, (long)(y1 + step_y * 2.));

    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, x2, (long)(y1 + step_y));
    cdCanvasLine (canvas, x1 - 1, (long)(y1 + step_y), x2 - 1, (long)(y1 + step_y * 2.));
    cdCanvasLine (canvas, x1, (long)(y1 + step_y * 2.), x2, (long)(y1 + step_y * 3.));
    cdCanvasLine (canvas, x1 + 1, (long)(y1 + 3. * step_y), x2 + 1, (long)(y1 + step_y * 4.));
    cdCanvasLine (canvas, x1, (long)(y1 + 4. * step_y), x2, y2);
    break;

  case TABS_RIGHT:
    cdCanvasForeground (canvas, tabs->mid_shadow);
    cdCanvasLine (canvas, x1, (long)(y1 + 3. * step_y), x2, (long)(y1 + step_y * 4.));
    cdCanvasLine (canvas, x1, (long)(y1 + step_y), x2, (long)(y1 + step_y * 2.));

    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, x2, (long)(y1 + step_y));
    cdCanvasLine (canvas, x1 - 1, (long)(y1 + step_y), x2 - 1, (long)(y1 + step_y * 2.));
    cdCanvasLine (canvas, x1, (long)(y1 + step_y * 2.), x2, (long)(y1 + step_y * 3.));
    cdCanvasLine (canvas, x1 + 1, (long)(y1 + 3. * step_y), x2 + 1, (long)(y1 + step_y * 4.));
    cdCanvasLine (canvas, x1, (long)(y1 + 4. * step_y), x2, y2);
    break;
  }

  cdCanvasForeground (canvas, old_color);
}

/*
* Desenha bordas
*/

static void DrawBorder (tTabs * tabs, long x1, long y1, long x2, long y2,
                        tTabsType type)
{
  cdCanvas *canvas = tabs->cddbuffer;
  switch (type)
  {
  case TABS_TOP:
    cdCanvasForeground (canvas, tabs->light_shadow);
    cdCanvasLine (canvas, x1, y1, x2, y2);
    break;
  case TABS_LEFT:
    cdCanvasForeground (canvas, tabs->light_shadow);
    cdCanvasLine (canvas, x1, y1, x2, y2);
    break;
  case TABS_BOTTOM:
    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, x2, y2);
    cdCanvasForeground (canvas, tabs->mid_shadow);
    if (x1>x2)
      cdCanvasLine (canvas, x2+1, y1+1, x1-1, y2+1);
    else
      cdCanvasLine (canvas, x1+1, y1+1, x2-1, y2+1);
    break;
  case TABS_RIGHT:
    cdCanvasForeground (canvas, tabs->dark_shadow);
    cdCanvasLine (canvas, x1, y1, x2, y2);
    cdCanvasForeground (canvas, tabs->mid_shadow);
    if (y1>y2)
      cdCanvasLine (canvas, x1-1, y2+1, x2-1, y1-1);
    else
      cdCanvasLine (canvas, x1-1, y1+1, x2-1, y2-1);
    break;
  }
}

/*
*  Desenha um canto de tab
*/ 

static void DrawCorner (tTabs * tabs, long color, int x1, int y1, int x2, int y2)
{
  cdCanvas *canvas = tabs->cddbuffer;
  cdCanvasForeground (canvas, tabs->bgcolor);
  cdCanvasLine (canvas, x1,y1,x2,y1);
  cdCanvasLine (canvas, x1,y1,x1,y2);
  cdCanvasForeground(canvas,color);
  cdCanvasLine (canvas, x1,y2,x2,y1);
}

/*
*  Desenha um tab
*/

static void DrawTab(tTabs * tabs, int tab_index, int offset)
{
  cdCanvas *canvas = tabs->cddbuffer;
  long x1, y1, x2, y2, x3, y3, x4, y4, x1a, y1a, x4a, y4a;
  long tab_w, tab_h;
  long x_text = 0, y_text = 0;
  long line_thick, tab_len, full_tab_len;
  tTabsType types[3];
  tBool beforeCurrent, afterCurrent;
  int extraHeight = (tab_index == tabs->current_tab) ? 0 : 1;
  tBrokenTabType type;

  /* there is an error in visible tabs that happen when you change the tab configuration dinamically */
  if (tab_index < 0 || tab_index > tabs->number_of_tabs-1)
    return;

  /* decide qual o tipo de tab a ser desenhado e seta largura de acordo  */
  
  full_tab_len = tabs->tabs_line.tabs_info[tab_index].tab_len;

  if (tab_index == (tabs->tabs_line.end_tab + 1))
  {
    type = BROKEN_END;
    tab_len = tabs->tabs_line.broken_space_end;
  }
  else if (tab_index == (tabs->tabs_line.start_tab - 1))
  {
    type = BROKEN_START;
    tab_len = tabs->tabs_line.broken_space_start;
  }
  else if (tabs->tabs_line.broken_center_visible == TABS_TRUE)
  {
    type = BROKEN_CENTER;
    tab_len = tabs->tabs_line.broken_tab_space;
  }
  else
  {
    type = BROKEN_NONE;
    tab_len = full_tab_len;
  }

  if (tab_len > full_tab_len)
    tab_len = full_tab_len;

  /*
  * Se tab selecionado nao for o primeiro e este for visivel,
  * subtrai um pixel dele e avanca tab em um pixel, para evitar que
  * lateral do tab se confunda com a borda do controle
  */

  if (tabs->current_tab != 0 &&
      ((tabs->tabs_line.start_tab == 0 && tab_index == 0) ||
       (tabs->tabs_line.start_tab > 0 && tab_index == (tabs->tabs_line.start_tab - 1))))
  {
    offset += 1;
    tab_len--;
    full_tab_len--;
  }

  line_thick = tabs->tabs_line.line_thick;

  if (tab_index != tabs->current_tab)
    line_thick -= TABS_CURRENT_EXTRA_PIXELS;

  if (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)
  {
    tab_w = line_thick;
    tab_h = tab_len;
  }
  else
  {
    tab_w = tab_len;
    tab_h = line_thick;
  }

  /*
  * calcula bordas dos tabs, posicao do texto e quais laterais serao
  * desenhadas
  */

  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    x1 = x1a = offset;
    y1 = tabs->h - *tabs->tabs_line.line_h - 1;
    y1a = y1 + extraHeight;
    x2 = x1;
    y2 = y1 + tab_h - 1;
    x3 = x2 + tab_w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;
    x4a = x3;
    y4a = y1;

    /* indica ordem de desenho */
    types[0] = TABS_LEFT;
    types[1] = TABS_TOP;
    types[2] = TABS_RIGHT;

    break;

  case TABS_BOTTOM:
    x1 = x1a = offset;
    y1 = *tabs->tabs_line.line_h - 1;
    y1a = y1 - extraHeight;
    x2 = x1;
    y2 = y1 - tab_h + 1;
    x3 = x2 + tab_w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;
    x4a = x3;
    y4a = y1;

    types[0] = TABS_LEFT;
    types[1] = TABS_BOTTOM;
    types[2] = TABS_RIGHT;

    break;

  case TABS_LEFT:
    x1 = *tabs->tabs_line.line_w - 1;
    x1a = x1 - extraHeight;
    y1 = y1a = tabs->h - offset - 1;
    x2 = x1 - tab_w + 1;
    y2 = y1;
    x3 = x2;
    y3 = y1 - tab_h + 1;
    x4 = x1;
    y4 = y3;
    x4a = x1a;
    y4a = y3;

    types[0] = TABS_TOP;
    types[1] = TABS_LEFT;
    types[2] = TABS_BOTTOM;

    break;

  case TABS_RIGHT:
    x1 = tabs->w - tabs->tabs_line.line_thick - 1;
    x1a = x1 + extraHeight;
    y1 = y1a = tabs->h - offset - 1;
    x2 = x1 + tab_w - 1;
    y2 = y1;
    x3 = x2;
    y3 = y1 - tab_h + 1;
    x4 = x1;
    y4 = y3;
    x4a = x1a;
    y4a = y3;

    types[0] = TABS_TOP;
    types[1] = TABS_RIGHT;
    types[2] = TABS_BOTTOM;

    break;

  default: 
    return;
  }

  if (tab_index == (tabs->current_tab - 1))
  {
    beforeCurrent = TABS_TRUE;
    afterCurrent = TABS_FALSE;
  }
  else if (tab_index == (tabs->current_tab + 1))
  {
    beforeCurrent = TABS_FALSE;
    afterCurrent = TABS_TRUE;
  }
  else
  {
    beforeCurrent = TABS_FALSE;
    afterCurrent = TABS_FALSE;
  }

  /* desenha quadrado do tab */

  cdCanvasForeground (canvas, tabs->bgcolor);

  /* preenche quadrado do tab */
  FillArea (canvas, x1, y1, x2, y2, x3, y3, x4, y4);

  /* desenha borda no "topo" */
  DrawBorder (tabs, x2, y2, x3, y3, types[1]);

  switch (type)
  {
  case BROKEN_END:
    switch (tabs->tabs_type)
    {
    case TABS_TOP:
      cdCanvasClipArea (canvas, x1, x3 - TABS_BORDER - 1, y1, y3);
      break;
    case TABS_BOTTOM:
      cdCanvasClipArea (canvas, x2, x4 - TABS_BORDER - 1, y2, y4);
      break;
    case TABS_LEFT:
      cdCanvasClipArea (canvas, x3, x1, y3 + TABS_BORDER + 1, y1);
      break;
    case TABS_RIGHT:
      cdCanvasClipArea (canvas, x4, x2, y4 + TABS_BORDER + 1, y2);
      break;
    }

    if (beforeCurrent == TABS_TRUE)
      DrawBrokenBorder (x1a, y1a, x2, y2, types[0], tabs);
    else if (afterCurrent == TABS_TRUE)
      DrawBrokenBorder (x3, y3, x4a, y4a, types[2], tabs);
    else
    {
      DrawBorder (tabs, x1a, y1a, x2, y2, types[0]);
      DrawBrokenBorder (x3, y3, x4a, y4a, types[2], tabs);
    }

    if(types[1] == TABS_TOP)
      DrawCorner ( tabs, tabs->light_shadow, x1 + TABS_BORDER - 1, y3 + TABS_BORDER - 1,
                                             x1 + TABS_BORDER + 1, y3 + TABS_BORDER - 3);

    if(types[1] == TABS_BOTTOM)
      DrawCorner ( tabs, tabs->dark_shadow, x2 + TABS_BORDER - 1, y2 + TABS_BORDER - 1,
                                            x2 + TABS_BORDER + 1, y2 + TABS_BORDER + 1);
    break;

  case BROKEN_START:
    switch (tabs->tabs_type)
    {
    case TABS_TOP:
      cdCanvasClipArea (canvas, x1 + TABS_BORDER + 1, x3, y1, y3);
      break;
    case TABS_BOTTOM:
      cdCanvasClipArea (canvas, x2 + TABS_BORDER + 1, x4, y2, y4);
      break;
    case TABS_LEFT:
      cdCanvasClipArea (canvas, x3, x1, y3, y1 - TABS_BORDER - 1);
      break;
    case TABS_RIGHT:
      cdCanvasClipArea (canvas, x4, x2, y4, y2 - TABS_BORDER - 1);
      break;
    }

    if (beforeCurrent == TABS_TRUE)
      DrawBrokenBorder (x1a, y1a, x2, y2, types[0], tabs);
    else if (afterCurrent == TABS_TRUE)
      DrawBrokenBorder (x3, y3, x4a, y4a, types[2], tabs);
    else
    {
      DrawBrokenBorder (x1a, y1a, x2, y2, types[0], tabs);
      DrawBorder (tabs, x3, y3, x4a, y4a, types[2]);
    }

    if(types[1] == TABS_TOP)
      DrawCorner ( tabs, tabs->dark_shadow, x3 + TABS_BORDER - 1, y3 + TABS_BORDER - 1,
                                            x3 + TABS_BORDER - 3, y3 + TABS_BORDER - 3);

    if(types[1] == TABS_BOTTOM)
      DrawCorner ( tabs, tabs->dark_shadow, x4 + TABS_BORDER - 1, y2 + TABS_BORDER - 1,
                                            x4 + TABS_BORDER - 3, y2 + TABS_BORDER + 1);
    break;

  case BROKEN_CENTER:
    switch (tabs->tabs_type)
    {
    case TABS_TOP:
      cdCanvasClipArea (canvas, x1 + TABS_BORDER + 1, x3 - TABS_BORDER - 1,
                  y1 + TABS_BORDER + 1, y3 - TABS_BORDER - 1);
      break;
    case TABS_BOTTOM:
      cdCanvasClipArea (canvas, x2 + TABS_BORDER + 1, x4 - TABS_BORDER - 1,
                  y2 + TABS_BORDER + 1, y4 - TABS_BORDER - 1);
      break;
    case TABS_LEFT:
      cdCanvasClipArea (canvas, x3 + TABS_BORDER + 1, x1 - TABS_BORDER - 1,
                  y3 + TABS_BORDER + 1, y1 - TABS_BORDER - 1);
      break;
    case TABS_RIGHT:
      cdCanvasClipArea (canvas, x4 + TABS_BORDER + 1, x2 - TABS_BORDER - 1,
                  y4 + TABS_BORDER + 1, y2 - TABS_BORDER - 1);
      break;
    }

    DrawBrokenBorder (x1, y1, x2, y2, types[0], tabs);
    DrawBrokenBorder (x3, y3, x4, y4, types[2], tabs);
    break;

  case BROKEN_NONE:
    switch (tabs->tabs_type)
    {
    case TABS_TOP:
      cdCanvasClipArea (canvas, x1 + TABS_BORDER + 1, x3 - TABS_BORDER - 1,
                  y1 + TABS_BORDER + 1, y3 - TABS_BORDER - 1);
      break;
    case TABS_BOTTOM:
      cdCanvasClipArea (canvas, x2 + TABS_BORDER + 1, x4 - TABS_BORDER - 1,
                  y2 + TABS_BORDER + 1, y4 - TABS_BORDER - 1);
      break;
    case TABS_LEFT:
      cdCanvasClipArea (canvas, x3 + TABS_BORDER + 1, x1 - TABS_BORDER - 1,
                  y3 + TABS_BORDER + 1, y1 - TABS_BORDER - 1);
      break;
    case TABS_RIGHT:
      cdCanvasClipArea (canvas, x4 + TABS_BORDER + 1, x2 - TABS_BORDER - 1,
                  y4 + TABS_BORDER + 1, y2 - TABS_BORDER - 1);
      break;
    }

    if (beforeCurrent == TABS_TRUE)
      DrawBorder (tabs, x1a, y1a, x2, y2, types[0]);
    else if (afterCurrent == TABS_TRUE)
      DrawBorder (tabs, x3, y3, x4a, y4a, types[2]);
    else
    {
      DrawBorder (tabs, x1a, y1a, x2, y2, types[0]);
      DrawBorder (tabs, x3, y3, x4a, y4a, types[2]);
    }

    if(types[1] == TABS_TOP)
    {
      DrawCorner ( tabs, tabs->light_shadow, x1 + TABS_BORDER - 1, y3 + TABS_BORDER - 1,
                                             x1 + TABS_BORDER + 1, y3 + TABS_BORDER - 3);

      DrawCorner ( tabs, tabs->dark_shadow, x3 + TABS_BORDER - 1, y3 + TABS_BORDER - 1,
                                            x3 + TABS_BORDER - 3, y3 + TABS_BORDER - 3);
    }

    if(types[1] == TABS_BOTTOM)
    {
      DrawCorner ( tabs, tabs->dark_shadow, x2 + TABS_BORDER - 1, y2 + TABS_BORDER - 1,
                                            x2 + TABS_BORDER + 1, y2 + TABS_BORDER + 1);

      DrawCorner ( tabs, tabs->dark_shadow, x4 + TABS_BORDER - 1, y2 + TABS_BORDER - 1,
                                            x4 + TABS_BORDER - 3, y2 + TABS_BORDER + 1);
    }

    if(types[1] == TABS_LEFT)
    {
      DrawCorner ( tabs, tabs->light_shadow, x3 + TABS_BORDER - 1, y1 + TABS_BORDER - 1,
                                             x3 + TABS_BORDER + 1, y1 + TABS_BORDER - 3);

      DrawCorner ( tabs, tabs->dark_shadow, x3 + TABS_BORDER - 1, y3 + TABS_BORDER - 1,
                                            x3 + TABS_BORDER + 1, y3 + TABS_BORDER + 1);
    }

    if(types[1] == TABS_RIGHT)
    {
      DrawCorner ( tabs, tabs->dark_shadow, x2 + TABS_BORDER - 1, y2 + TABS_BORDER - 1,
                                            x2 + TABS_BORDER - 3, y2 + TABS_BORDER - 3);

      DrawCorner ( tabs, tabs->dark_shadow, x2 + TABS_BORDER - 1, y4 + TABS_BORDER - 1,
                                            x2 + TABS_BORDER - 3, y4 + TABS_BORDER + 1);
    }

    break;
  }

  if (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)
    tab_h = full_tab_len;
  else
    tab_w = full_tab_len;

  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    x_text = x1 + tab_w / 2;
    y_text = y1 + tab_h / 2;
    break;
  case TABS_BOTTOM:
    x_text = x2 + tab_w / 2;
    y_text = y2 + tab_h / 2;
    break;
  case TABS_LEFT:
    x_text = x1 - tab_w / 2;
    y_text = y1 - tab_h / 2;
    break;
  case TABS_RIGHT:
    x_text = x2 - tab_w / 2;
    y_text = y2 - tab_h / 2;
    break;
  }
  cdCanvasTextAlignment (canvas, CD_CENTER);

  /* desenha texto */

  cdCanvasForeground (canvas, tabs->fgcolor); 

  if (tabs->tabs_orientation == TABS_VERTICAL)
    cdCanvasTextOrientation(canvas, 90);
  else
    cdCanvasTextOrientation(canvas, 0);

  {
    char* tab_font = NULL;
    int bold = 0;

    if (iupStrEqualNoCase(IupGetAttribute(tabs->tabs_line.tabs_info[tab_index].ihandle,IUP_ACTIVE), IUP_YES))
    {
      if (tab_index == tabs->current_tab)
      {
        tab_font = tabs->fontact;
        bold = 1;
      }
    }
    else
    {
      cdCanvasForeground(canvas, CD_DARK_GRAY);
      tab_font = tabs->fontinact;
    }

    setcdfont(tabs, tab_font, bold);
  }

  cdCanvasClip(canvas, CD_CLIPAREA);

  {
    char* text = iupGetEnv(tabs->tabs_line.tabs_info[tab_index].ihandle, ICTL_TABTITLE);
    if (text)
      cdCanvasText(canvas, x_text, y_text, text);
  }

  cdCanvasClip (canvas, CD_CLIPOFF);
}

/*
*  Desenha borda "inferior" dos labels dos tabs
*/

static void DrawLabelLowerBorder (tTabs * tabs, long start, long end)
{
  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    DrawBorder (tabs, start, tabs->h - *tabs->tabs_line.line_h - 1 - 1,
                      end, tabs->h - *tabs->tabs_line.line_h - 1 - 1, TABS_TOP);
    break;

  case TABS_LEFT:
    if(!start)
      DrawBorder (tabs, *tabs->tabs_line.line_w - 1 + 1, tabs->h - 1 - start,
                        *tabs->tabs_line.line_w - 1 + 1, tabs->h - 1 - end, TABS_LEFT);
    else
      DrawBorder (tabs, *tabs->tabs_line.line_w - 1 + 1, tabs->h - 1 - start,
                        *tabs->tabs_line.line_w - 1 + 1, 0, TABS_LEFT);
    break;

  case TABS_BOTTOM:
    DrawBorder (tabs, start, *tabs->tabs_line.line_h - 1 + 1,
                      end, *tabs->tabs_line.line_h - 1 + 1, TABS_BOTTOM);
    break;

  case TABS_RIGHT:
    if(!start)
      DrawBorder (tabs, tabs->w - *tabs->tabs_line.line_w - 1 - 1, tabs->h - 1 - start,
                        tabs->w - *tabs->tabs_line.line_w - 1 - 1, tabs->h - 1 - end, TABS_RIGHT);
    else
      DrawBorder (tabs, tabs->w - *tabs->tabs_line.line_w - 1 - 1, tabs->h - 1 - start,
                        tabs->w - *tabs->tabs_line.line_w - 1 - 1, 0, TABS_RIGHT);
    break;
  }
}

/*
*  Desenha botao normal
*/

static void DrawButton (tTabs * tabs, int x, int y, int width, int height)
{
  cdCanvas *canvas = tabs->cddbuffer;
  long old_color = cdCanvasForeground (canvas, tabs->bgcolor);

  cdCanvasBegin (canvas, CD_FILL);
  cdCanvasVertex (canvas, x, y);
  cdCanvasVertex (canvas, x + width - 1, y);
  cdCanvasVertex (canvas, x + width - 1, y + height - 1);
  cdCanvasVertex (canvas, x, y + height - 1);
  cdCanvasEnd (canvas);

  cdCanvasForeground (canvas, old_color);

  DrawBorder (tabs, x + width - 1, y + height - 1, x, y + height - 1, TABS_TOP);
  DrawBorder (tabs, x, y + height - 1, x, y, TABS_LEFT);
  DrawBorder (tabs, x, y, x + width - 1, y, TABS_BOTTOM);
  DrawBorder (tabs, x + width - 1, y, x + width - 1, y + height - 1, TABS_RIGHT);
}

/*
*  Desenha seta
*
*  x,y -> canto inferior da bounding box
*  width, height -> dimensoes da bounding box
*  type -> TABS_LEFT, TABS_RIGHT, TABS_TOP, TABS_BOTTOM
*/

static void DrawArrow (tTabs *tabs, int x, int y, int width, int height, int type)
{
  cdCanvas *canvas = tabs->cddbuffer;
  long old_color = cdCanvasForeground (canvas, CD_BLACK);

  cdCanvasBegin (canvas, CD_CLOSED_LINES);

  switch (type)
  {
  case TABS_LEFT:
    cdCanvasVertex (canvas, x + 4, y + height / 2);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + 4);
    break;

  case TABS_RIGHT:
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height / 2);
    cdCanvasVertex (canvas, x + 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + 4, y + 4);
    break;

  case TABS_TOP:
    cdCanvasVertex (canvas, x + 4, y + 4);
    cdCanvasVertex (canvas, x + width / 2, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + 4);
    break;

  case TABS_BOTTOM:
    cdCanvasVertex (canvas, x + 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width / 2, y + 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height - 1 - 4);
    break;
  }

  cdCanvasEnd (canvas);

  cdCanvasBegin (canvas, CD_FILL);

  switch (type)
  {
  case TABS_LEFT:
    cdCanvasVertex (canvas, x + 4, y + height / 2);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + 4);
    break;

  case TABS_RIGHT:
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height / 2);
    cdCanvasVertex (canvas, x + 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + 4, y + 4);
    break;

  case TABS_TOP:
    cdCanvasVertex (canvas, x + 4, y + 4);
    cdCanvasVertex (canvas, x + width / 2, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + 4);
    break;

  case TABS_BOTTOM:
    cdCanvasVertex (canvas, x + 4, y + height - 1 - 4);
    cdCanvasVertex (canvas, x + width / 2, y + 4);
    cdCanvasVertex (canvas, x + width - 1 - 4, y + height - 1 - 4);
    break;
  }

  cdCanvasEnd (canvas);

  cdCanvasForeground (canvas, old_color);
}

/*
*  Desenha "..."
*/

static void DrawDots (tTabs *tabs, int x, int y, int width, int height)
{
  x -= 1;
  y += height / 3 - 1;
  cdCanvasRect (tabs->cddbuffer, x + 3, x+4, y+3, y + 4);
  cdCanvasRect (tabs->cddbuffer, x + width / 2, x+ width/2 +1 , y+3, y + 4);
  cdCanvasRect (tabs->cddbuffer, x + width - 1 - 3, x + width -1 - 2, y +3, y + 4);
}

/*
*  Desenha scrollbar_visible com botao "..."
*/

static void DrawScrollbar (tTabs * tabs)
{
  long x = tabs->tabs_line.scroll_x,
       y = tabs->tabs_line.scroll_y,
       w = tabs->tabs_line.scroll_w,
       h = tabs->tabs_line.scroll_h;
  int scroll_updown;

  if (tabs->tabs_type == TABS_TOP ||
      tabs->tabs_type == TABS_BOTTOM)
    scroll_updown = 0;
  else
    scroll_updown = 1;

  if (tabs->tabs_orientation == TABS_HORIZONTAL)
  {
    int x3 = w / 3;

    DrawButton(tabs, x, y, x3,  h );
    DrawArrow(tabs,  x, y, x3, h, scroll_updown? TABS_TOP: TABS_LEFT);

    DrawButton(tabs, x + x3, y, x3, h );
    DrawDots(tabs,   x + x3, y, x3, h);

    DrawButton(tabs, x + 2 * x3, y, x3, h );
    DrawArrow(tabs,  x + 2 * x3, y, x3, h, scroll_updown? TABS_BOTTOM: TABS_RIGHT);
  }
  else
  {
    int y3 = h / 3;

    DrawButton(tabs, x, y, w,  y3 );
    DrawArrow(tabs,  x, y, w, y3, scroll_updown? TABS_BOTTOM: TABS_RIGHT);

    DrawButton(tabs, x, y + y3, w,  y3 );
    DrawDots(tabs,   x, y + y3, w,  y3);

    DrawButton(tabs, x, y + 2 * y3, w,  y3 );
    DrawArrow(tabs,  x, y + 2 * y3, w,  y3, scroll_updown? TABS_TOP: TABS_LEFT);
  }

  DrawCorner(tabs, tabs->dark_shadow, x + w - 1, y + h  - 1, x + w - 3, y + h - 3);
  DrawCorner(tabs, tabs->dark_shadow, x + w - 1, y, x + w - 3, y + 2);
  DrawCorner(tabs, tabs->light_shadow, x, y + h - 1, x + 2, y + h - 3);
  DrawCorner(tabs, tabs->dark_shadow, x, y, x + 2, y + 2);
}

/*
* Limpa background do tab
*/

static void ClearBackground (tTabs * tabs)
{
  long x1, y1, x2, y2, x3, y3, x4, y4;
  cdCanvas *canvas = tabs->cddbuffer;

  /* esta cor e´ a do parent necessariamente ? */
  cdCanvasBackground (canvas, cdIupConvertColor(iupGetParentBgColor(tabs->self)));
  cdCanvasClear (canvas);

  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    x1 = 0;
    y1 = tabs->h - *tabs->tabs_line.line_h - 1 - 1;
    x2 = x1;
    y2 = 0;
    x3 = tabs->w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;
    break;

  case TABS_BOTTOM:
    x1 = 0;
    y1 = *tabs->tabs_line.line_h - 1 + 1;
    x2 = x1;
    y2 = tabs->h - 1;
    x3 = tabs->w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;
    break;

  case TABS_LEFT:
    x1 = *tabs->tabs_line.line_w - 1 + 1;
    y1 = tabs->h - 1;
    x2 = tabs->w - 1;
    y2 = y1;
    x3 = x2;
    y3 = 0;
    x4 = x1;
    y4 = y3;
    break;

  case TABS_RIGHT:
    x1 = tabs->w - *tabs->tabs_line.line_w - 1 - 1;
    y1 = tabs->h - 1;
    x2 = 0;
    y2 = y1;
    x3 = x2;
    y3 = 0;
    x4 = x1;
    y4 = y3;
    break;

  default:
    return;
  }

  /* cor interna */
  cdCanvasForeground (canvas, tabs->bgcolor);
  cdCanvasBegin (canvas, CD_FILL);
  cdCanvasVertex (canvas, x1, y1);
  cdCanvasVertex (canvas, x2, y2);
  cdCanvasVertex (canvas, x3, y3);
  cdCanvasVertex (canvas, x4, y4);
  cdCanvasEnd (canvas);
  cdCanvasBegin (canvas, CD_CLOSED_LINES);
  cdCanvasVertex (canvas, x1, y1);
  cdCanvasVertex (canvas, x2, y2);
  cdCanvasVertex (canvas, x3, y3);
  cdCanvasVertex (canvas, x4, y4);
  cdCanvasEnd (canvas);
}

/*
* Desenha bordas do tab
*/

static void DrawTabsBorders (tTabs * tabs)
{
  long x1, y1, x2, y2, x3, y3, x4, y4;
  tTabsType types[3];

  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    x1 = 0;
    y1 = tabs->h - *tabs->tabs_line.line_h - 1 - 1;
    x2 = x1;
    y2 = 0;
    x3 = tabs->w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;

    /* indica ordem de desenho */
    types[0] = TABS_LEFT;
    types[1] = TABS_BOTTOM;
    types[2] = TABS_RIGHT;
    break;

  case TABS_BOTTOM:
    x1 = 0;
    y1 = *tabs->tabs_line.line_h - 1 + 1;
    x2 = x1;
    y2 = tabs->h - 1;
    x3 = tabs->w - 1;
    y3 = y2;
    x4 = x3;
    y4 = y1;

    /* indica ordem de desenho */
    types[0] = TABS_LEFT;
    types[1] = TABS_TOP;
    types[2] = TABS_RIGHT;
    break;

  case TABS_LEFT:
    x1 = *tabs->tabs_line.line_w - 1 + 1;
    y1 = tabs->h - 1;
    x2 = tabs->w - 1;
    y2 = y1;
    x3 = x2;
    y3 = 0;
    x4 = x1;
    y4 = y3;

    /* indica ordem de desenho */
    types[0] = TABS_TOP;
    types[1] = TABS_RIGHT;
    types[2] = TABS_BOTTOM;
    break;

  case TABS_RIGHT:
    x1 = tabs->w - *tabs->tabs_line.line_w - 1 - 1;
    y1 = tabs->h - 1;
    x2 = 0;
    y2 = y1;
    x3 = x2;
    y3 = 0;
    x4 = x1;
    y4 = y3;

    /* indica ordem de desenho */
    types[0] = TABS_TOP;
    types[1] = TABS_LEFT;
    types[2] = TABS_BOTTOM;
    break;

  default:
    return;
  }

  DrawBorder (tabs, x1, y1, x2, y2, types[0]);
  DrawBorder (tabs, x2, y2, x3, y3, types[1]);
  DrawBorder (tabs, x3, y3, x4, y4, types[2]);
}

/*
* Desenha todos os tabs e a scrollbar_visible (se houver)
*/

static void DrawLineOfTabs (tTabs * tabs)
{
  long offset = 0;
  int c = 0;

  /* desenha decoracoes, se necessario */

  if (tabs->tabs_line.broken_start_visible == TABS_TRUE)
  {
    DrawTab (tabs, tabs->tabs_line.start_tab - 1, offset);
    offset += tabs->tabs_line.broken_space_start;
  }

  if (tabs->tabs_line.broken_center_visible == TABS_FALSE)
  {
    for (c = tabs->tabs_line.start_tab; c <= tabs->tabs_line.end_tab; c++)
    {
      if (c != tabs->current_tab)
      {
        DrawTab (tabs, c, offset);
        offset += tabs->tabs_line.tabs_info[c].tab_len;
      }
      else
      { 
        if (tabs->current_tab != 0)
          DrawLabelLowerBorder (tabs, 0, offset);

        DrawTab (tabs, c, offset);
        offset += tabs->tabs_line.tabs_info[c].tab_len;
        DrawLabelLowerBorder (tabs, offset - 1, tabs->w);
      }
    }
  }
  else
  {
    if (tabs->current_tab != 0)
      DrawLabelLowerBorder (tabs, 0, offset);
    DrawTab (tabs, tabs->current_tab, offset);
    offset += tabs->tabs_line.broken_tab_space;
    DrawLabelLowerBorder (tabs, offset, tabs->w);
  }

  if (tabs->tabs_line.broken_end_visible)
  {
    DrawTab (tabs, tabs->tabs_line.end_tab + 1, offset);
    offset += tabs->tabs_line.broken_space_end;
  }

  if (tabs->tabs_line.scrollbar_visible == TABS_TRUE)
  {
    DrawScrollbar(tabs);
  }
}


/*
*  Chama callback de mudanca de tabs
*/

static int cb_tabchange (tTabs * tabs, Ihandle * new_tab, Ihandle * old_tab)
{
  IFnnn user_callback = NULL;
  if(iupStrEqual(IupGetAttribute(new_tab, IUP_ACTIVE), IUP_NO)) return IUP_IGNORE;

  user_callback = (IFnnn)IupGetCallback(tabs->self, ICTL_TABCHANGE_CB);
  if (user_callback != NULL)
    return (user_callback) (tabs->self, new_tab, old_tab);

  return IUP_DEFAULT;
}

/*
*  Muda value da ZBOX e redesenha tabs
*/

static void SetNewCurrentTab (tTabs * tabs)
{
  char *next_tab_name = NULL;

  if (tabs->cddbuffer)
    doRepaint(tabs);

  next_tab_name = IupGetName (GetTabIhandle (tabs, tabs->current_tab));

  assert (next_tab_name);
  if (next_tab_name == NULL)
    return;

  IupSetAttribute (tabs->zbox, IUP_VALUE, next_tab_name);
}

/**************************************
* funcoes de calculo de tamanho e de *
* visibilidade dos tabs              *
**************************************/

/* calcula o tamanho da caixa que envolve o texto das abas dos tabs */

static void calc_text_size(tTabs *tabs)
{
  char *title;
  int t, text_w, size, tabsize;
  int text_width, text_height;

  iupSetEnv(tabs->self, "_IUPTABS_RESIZE_FIRST_TIME", NULL);

  iupdrvStringSize(tabs->self, "Âj", NULL, &text_height);
  tabs->tabs_line.text_h = text_height;

  /* Pega o tamanho global do tab */
  size = iupGetEnvInt(tabs->self, ICTL_TABSIZE);

  for(t = 0; t < tabs->number_of_tabs; t++)
  {
    Ihandle* tab_child = tabs->tabs_line.tabs_info[t].ihandle;

    title = iupGetEnv(tab_child, ICTL_TABTITLE);
    if (!title) 
    {
      title = "   ";
      iupSetEnv(tab_child, ICTL_TABTITLE, "   ");
    }

    /* Pega o tamanho de um tab especifico */   
    tabsize = iupGetEnvInt(tab_child, ICTL_TABSIZE);
    iupdrvStringSize(tabs->self, title, &text_width, NULL);

    /* Utiliza o tamanho especifico se for maior do que o tamanho minimo */
    if (tabsize > text_width)
      text_w = tabsize;
    else
    {
      /* Senao, utiliza o tamanho especifico se for maior do que o minimo */
      if(size > text_width)
        text_w = size;
      else /* Caso contrario, utiliza o tamanho minimo */
        text_w = text_width;
    }

    tabs->tabs_line.tabs_info[t].text_w = text_w;
  }
}

/*
*  Calcula tamanho de cada um dos tabs
*/

static void calc_tabs_size (tTabs * tabs)
{
  int t, tab_len, tab_thick;

  calc_text_size(tabs);

  tabs->tabs_line.line_thick = 0;
  tabs->tabs_line.line_length = 0;

  for (t = 0; t < tabs->number_of_tabs; t++)
  {
    /* initialize with the text size */
    if (tabs->tabs_render == RENDER_PLANE_ALIGN)
    {
      tab_len = tabs->tabs_line.tabs_info[t].text_w;
      tab_thick = tabs->tabs_line.text_h;
    }
    else
    {
      tab_len = tabs->tabs_line.text_h;
      tab_thick = tabs->tabs_line.tabs_info[t].text_w;
    }

    /* add the borders */
    
    if (abs(tabs->current_tab - t) == 1) /* if neighbour of the current tab */
      tab_len += TABS_BORDER;
    else if (t == tabs->current_tab) /* if tab is the current tab */
    {
      if (tabs->current_tab == 0 || tabs->current_tab == (tabs->number_of_tabs - 1))
        tab_len += 2 * TABS_BORDER;
      else
        tab_len += 3 * TABS_BORDER;
    }
    else
      tab_len += 2 * TABS_BORDER;

    /* add margins */
    if (tabs->tabs_render == RENDER_PLANE_ALIGN)
      tab_len += 2 * TABS_MARGIN;
    else
      tab_len += 2 * TABS_SPACING;

    tabs->tabs_line.tabs_info[t].tab_len = tab_len;

    tabs->tabs_line.line_length += tab_len;      /* is always the sum on the lengths */
    if (tab_thick > tabs->tabs_line.line_thick)  /* is always the maximum thickness */
      tabs->tabs_line.line_thick = tab_thick;
  }

  if (tabs->tabs_render == RENDER_PLANE_ALIGN)
    tabs->tabs_line.line_thick += 2 * TABS_SPACING + 2 * TABS_BORDER;
  else
    tabs->tabs_line.line_thick += 2 * TABS_MARGIN + 2 * TABS_BORDER;

  if(tabs->tabs_line.line_thick < tabs->tabs_line.scroll_thick) 
    tabs->tabs_line.line_thick = tabs->tabs_line.scroll_thick;

  /* pixels a mais do tab selecionado */
  tabs->tabs_line.line_thick += TABS_CURRENT_EXTRA_PIXELS;
}

void calc_scroll_pos(tTabs * tabs)
{
  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    tabs->tabs_line.scroll_x = tabs->w - tabs->tabs_line.scroll_w - 2;
    tabs->tabs_line.scroll_y = tabs->h - *tabs->tabs_line.line_h;
    break;
  case TABS_BOTTOM:
    tabs->tabs_line.scroll_x = tabs->w - tabs->tabs_line.scroll_w - 2;
    tabs->tabs_line.scroll_y = *tabs->tabs_line.line_h - 2 - tabs->tabs_line.scroll_h + 1;
    break;
  case TABS_LEFT:
    tabs->tabs_line.scroll_x = *tabs->tabs_line.line_w - 2 - tabs->tabs_line.scroll_w + 1; 
    tabs->tabs_line.scroll_y = 0;
    break;                               
  case TABS_RIGHT:
    tabs->tabs_line.scroll_x = tabs->w - *tabs->tabs_line.line_w;
    tabs->tabs_line.scroll_y = 0;
    break;
  }
}

/*
* calcula visibilidade dos tabs
*/

static void visible_tabs (tTabs * tabs)
{
  long line_length = *tabs->max_line_length;        
  long old_line_length;
  int t, t_aux = 0;

  /* verifica se todos os tabs estao visiveis */
  if (*tabs->max_line_length > tabs->tabs_line.line_length)
  {
    /* todos os tabs sao visiveis */
    tabs->tabs_line.scrollbar_visible = TABS_FALSE;
    tabs->tabs_line.broken_start_visible = TABS_FALSE;
    tabs->tabs_line.broken_end_visible = TABS_FALSE;
    tabs->tabs_line.broken_center_visible = TABS_FALSE;

    tabs->tabs_line.start_tab = 0;
    tabs->tabs_line.end_tab = tabs->number_of_tabs - 1;

    return;
  }

  /* subtrai espaco das scrollbars */

  line_length -= tabs->tabs_line.scroll_len + TABS_SCROLL_SPACING;

  tabs->tabs_line.scrollbar_visible = TABS_TRUE;

  /* Verifica se current tab e' menor que start tab e acerta caso seja */
  if (tabs->current_tab < tabs->tabs_line.start_tab)
    tabs->tabs_line.start_tab = tabs->current_tab;

  /*
  * processa tabs em sequencia, ate que "largura" total ultrapasse
  * largura virtual
  */

  t = tabs->tabs_line.start_tab;
  old_line_length = line_length;

  while (1)
  {
    do
    {
      while (tabs->tabs_line.start_tab < tabs->current_tab)
      {
        if (tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab].tab_len <= line_length)
            break;
        tabs->tabs_line.start_tab++;
      }

      for (t = tabs->tabs_line.start_tab, t_aux = 0;
           t < tabs->number_of_tabs; t++, t_aux++)
      {
        if (tabs->tabs_line.tabs_info[t].tab_len > line_length)
        {
          if ((t - 1) < tabs->current_tab)
          {
            while (t_aux > 0)
            {
              line_length +=
                tabs->tabs_line.tabs_info[t - t_aux].tab_len;

              if (tabs->tabs_line.start_tab == 0)
              {
                line_length -= TABS_BROKEN_TAB;
                tabs->tabs_line.broken_start_visible = TABS_TRUE;
              }

              tabs->tabs_line.start_tab++;

              if (tabs->tabs_line.tabs_info[t].tab_len <= line_length)
                break;

              t_aux--;
            }

            if (tabs->tabs_line.tabs_info[t].tab_len > line_length)        
            {
              /* nao conseguiu */
              tabs->tabs_line.broken_center_visible = TABS_TRUE;
              break;                /* sai dos dois loops */
            }
          }
          else
          {
            t--;
            break;                /* sai dos dois loops */
          }
        }

        line_length -= tabs->tabs_line.tabs_info[t].tab_len;
        tabs->tabs_line.broken_center_visible = TABS_FALSE;
      }

      if (t == tabs->number_of_tabs)
        t--;
    }
    while (t < tabs->current_tab);

    if (tabs->tabs_line.start_tab > 0 &&
        tabs->tabs_line.broken_center_visible != TABS_TRUE)
    {
      if (tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab - 1].tab_len <
          (line_length - TABS_BROKEN_TAB))
      {
        tabs->tabs_line.start_tab--;
        line_length = old_line_length;
        continue;
      }
    }

    break;
  }

  if(tabs->tabs_line.start_tab > 0)
  {
    t = tabs->tabs_line.start_tab;
    line_length = old_line_length;
    line_length -= TABS_BROKEN_TAB;
    tabs->tabs_line.broken_start_visible = TABS_TRUE;

    while (1)
    {
      do
      {
        while (tabs->tabs_line.start_tab < tabs->current_tab)
        {
          if (tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab].tab_len <= line_length)
              break;
          tabs->tabs_line.start_tab++;
        }

        for (t = tabs->tabs_line.start_tab, t_aux = 0;
             t < tabs->number_of_tabs; t++, t_aux++)
        {
          if (tabs->tabs_line.tabs_info[t].tab_len > line_length)
          {
            if ((t - 1) < tabs->current_tab)
            {
              while (t_aux > 0)
              {
                line_length +=
                  tabs->tabs_line.tabs_info[t - t_aux].tab_len;

                if (tabs->tabs_line.start_tab == 0)
                {
                  line_length -= TABS_BROKEN_TAB;
                  tabs->tabs_line.broken_start_visible = TABS_TRUE;
                }

                tabs->tabs_line.start_tab++;

                if (tabs->tabs_line.tabs_info[t].tab_len <= line_length)
                  break;

                t_aux--;
              }

              if (tabs->tabs_line.tabs_info[t].tab_len > line_length)
              {
                /* nao conseguiu */
                tabs->tabs_line.broken_center_visible = TABS_TRUE;
                break;                /* sai dos dois loops */
              }
            }
            else
            {
              t--;
              break;                /* sai dos dois loops */
            }
          }

          line_length -= tabs->tabs_line.tabs_info[t].tab_len;
          tabs->tabs_line.broken_center_visible = TABS_FALSE;
        }

        if (t == tabs->number_of_tabs)
          t--;
      }
      while (t < tabs->current_tab);

      if (tabs->tabs_line.start_tab > 0 &&
          tabs->tabs_line.broken_center_visible != TABS_TRUE)
      {
        if (tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab - 1].tab_len <
            (line_length - TABS_BROKEN_TAB))
        {
          tabs->tabs_line.start_tab--;
          line_length = old_line_length;
          continue;
        }
      }

      break;

    }
  }
  else
    tabs->tabs_line.broken_start_visible = TABS_FALSE;

  /*
  * espaco que nao foi ocupado por nenhum tab e' destinado aos
  * broken_tabs
  */
  tabs->tabs_line.broken_tab_space = line_length;

  if (tabs->tabs_line.broken_start_visible == TABS_TRUE)
    tabs->tabs_line.broken_tab_space += TABS_BROKEN_TAB;

  if (tabs->tabs_line.broken_center_visible == TABS_TRUE &&
      tabs->tabs_line.start_tab == t)
  {
    /* se so' houver um tab visivel e mais tabs `a direita, 
       ele sera clipado */
    tabs->tabs_line.end_tab = tabs->tabs_line.start_tab;
    tabs->tabs_line.broken_end_visible = TABS_FALSE;
    tabs->tabs_line.broken_start_visible = TABS_FALSE;
  }
  else
  {
    tabs->tabs_line.end_tab = t;
    tabs->tabs_line.broken_center_visible = TABS_FALSE;

    /*
    * se ultimo tab nao estiver visivel, deve-se incluir o broken
    * tab direito e eliminar um dos tabs, se necessario
    */

    if (tabs->tabs_line.end_tab < (tabs->number_of_tabs - 1))
    {
      tabs->tabs_line.broken_end_visible = TABS_TRUE;

      if (line_length < TABS_BROKEN_TAB)
      {
        if (tabs->tabs_line.start_tab == tabs->tabs_line.end_tab)
        {                        /* nao ha espaco para broken tabs */
          tabs->tabs_line.broken_center_visible = TABS_TRUE;
          tabs->tabs_line.broken_end_visible = TABS_FALSE;
          tabs->tabs_line.broken_start_visible = TABS_FALSE;

          tabs->tabs_line.broken_tab_space +=
            tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab].tab_len;
        }
        else if (tabs->current_tab == tabs->tabs_line.end_tab)
        {
          tabs->tabs_line.broken_tab_space +=
            tabs->tabs_line.tabs_info[tabs->tabs_line.start_tab].tab_len;
          tabs->tabs_line.start_tab++;
          tabs->tabs_line.broken_start_visible = TABS_TRUE;
        }
        else
        {
          tabs->tabs_line.broken_tab_space +=
            tabs->tabs_line.tabs_info[tabs->tabs_line.end_tab].tab_len;
          tabs->tabs_line.end_tab--;
        }
      }
    }
    else
      tabs->tabs_line.broken_end_visible = TABS_FALSE;
  }

  if (tabs->tabs_line.broken_start_visible == TABS_TRUE &&
      tabs->tabs_line.broken_end_visible == TABS_TRUE)
  {
    tabs->tabs_line.broken_space_start = (int)
      ceil ((double) tabs->tabs_line.broken_tab_space / 2);
    tabs->tabs_line.broken_space_end = (int)
      floor ((double) tabs->tabs_line.broken_tab_space / 2);
  }
  else if (tabs->tabs_line.broken_start_visible == TABS_TRUE)
  {
    tabs->tabs_line.broken_space_start = tabs->tabs_line.broken_tab_space;
    tabs->tabs_line.broken_space_end = 0;
  }
  else if (tabs->tabs_line.broken_end_visible == TABS_TRUE)
  {
    tabs->tabs_line.broken_space_end = tabs->tabs_line.broken_tab_space;
    tabs->tabs_line.broken_space_start = 0;
  }
  else
  {
    tabs->tabs_line.broken_space_end = 0;
    tabs->tabs_line.broken_space_start = 0;
  }

  if (tabs->tabs_line.start_tab == 0)
    tabs->tabs_line.broken_start_visible = TABS_FALSE;
}

/*
* Calcula tamanhos de acordo com o tamanho da area cliente
* (zbox)
*/

static void calcsize_by_client (tTabs * tabs)
{
  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    tabs->w = tabs->client_w + 2 * TABS_MARGIN;
    tabs->h = tabs->client_h + *tabs->tabs_line.line_h + 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = *tabs->tabs_line.line_h + TABS_MARGIN;
    break;

  case TABS_BOTTOM:
    tabs->w = tabs->client_w + 2 * TABS_MARGIN;
    tabs->h = tabs->client_h + *tabs->tabs_line.line_h + 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;

  case TABS_LEFT:
    tabs->w = tabs->client_w + *tabs->tabs_line.line_w + 2 * TABS_MARGIN;
    tabs->h = tabs->client_h + 2 * TABS_MARGIN;
    tabs->client_x = *tabs->tabs_line.line_w + TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;

  case TABS_RIGHT:
    tabs->w = tabs->client_w + *tabs->tabs_line.line_w + 2 * TABS_MARGIN;
    tabs->h = tabs->client_h + 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;
  }
}

/*
* Calcula tamanhos de acordo com o tamanho do canvas CPI
*/

static void calcsize_by_canvas (tTabs * tabs)
{
  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    tabs->client_w = tabs->w - 2 * TABS_MARGIN;
    tabs->client_h = tabs->h - *tabs->tabs_line.line_h - 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = *tabs->tabs_line.line_h + TABS_MARGIN;
    break;

  case TABS_BOTTOM:
    tabs->client_w = tabs->w - 2 * TABS_MARGIN;
    tabs->client_h = tabs->h - *tabs->tabs_line.line_h - 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;

  case TABS_LEFT:
    tabs->client_w = tabs->w - *tabs->tabs_line.line_w - 2 * TABS_MARGIN;
    tabs->client_h = tabs->h - 2 * TABS_MARGIN;
    tabs->client_x = *tabs->tabs_line.line_w + TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;

  case TABS_RIGHT:
    tabs->client_w = tabs->w - *tabs->tabs_line.line_w - 2 * TABS_MARGIN;
    tabs->client_h = tabs->h - 2 * TABS_MARGIN;
    tabs->client_x = TABS_MARGIN;
    tabs->client_y = TABS_MARGIN;
    break;
  }
}

/***************
* Metodos CPI *
***************/

static int tabssetnaturalsize (Ihandle * self)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  assert (tabs != NULL);
  if (tabs == NULL)
    return 3;

  iupCpiSetNaturalSize (tabs->zbox);

  tabs->client_w = iupGetNaturalWidth (tabs->zbox);
  tabs->client_h = iupGetNaturalHeight (tabs->zbox);

  calc_tabs_size(tabs);

  calcsize_by_client(tabs);        
  calc_scroll_pos(tabs);
                                    
  iupSetNaturalWidth (self, tabs->w);
  iupSetNaturalHeight (self, tabs->h);

  return 3;
}

static void tabssetcurrentsize (Ihandle * self, int max_w, int max_h)
{
  tTabs *tabs = (tTabs *) iupGetEnv(self, "_IUPTABS_DATA");
  assert(tabs != NULL);
  if (tabs == NULL)
    return;

  iupSetCurrentWidth(self, tabs->w = max_w);
  iupSetCurrentHeight(self, tabs->h = max_h);

  calc_tabs_size(tabs);

  calcsize_by_canvas(tabs);
  calc_scroll_pos(tabs);

  iupCpiSetCurrentSize (tabs->zbox, tabs->client_w, tabs->client_h);
}

static void tabssetposition (Ihandle * self, int x, int y)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  assert (tabs != NULL);
  if (tabs == NULL)
    return;

  iupSetPosX (self, x);
  iupSetPosY (self, y);

  iupCpiSetPosition (tabs->zbox, tabs->client_x, tabs->client_y);
  iupdrvResizeObjects (tabs->zbox);
}

static void tabs_createcanvas(tTabs *tabs)
{
  if (!tabs->cdcanvas)
    tabs->cdcanvas = cdCreateCanvas (CD_IUP, tabs->self);

  if (tabs->cdcanvas)
  {
    cdCanvasActivate(tabs->cdcanvas);
    tabs->cddbuffer = cdCreateCanvas(CD_DBUFFER,tabs->cdcanvas);
  }
}

static int resize_cb (Ihandle * self)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassName(self), "tabs"))
    return IUP_IGNORE;

  assert(tabs != NULL);
  if (tabs == NULL)
    return IUP_DEFAULT;

  /* verifica se e' um evento do canvas do tabs */
  if (self != tabs->self)
    return IUP_DEFAULT;

  if (!tabs->cddbuffer)
    tabs_createcanvas(tabs);

  assert(tabs->cddbuffer != NULL);
  if (!tabs->cddbuffer)
    return IUP_DEFAULT;

  cdCanvasActivate(tabs->cddbuffer);

  if (iupGetEnvRaw(tabs->self, "_IUPTABS_RESIZE_FIRST_TIME"))
    calc_tabs_size(tabs);

  return IUP_DEFAULT;
}

static void doRepaint(tTabs *tabs)
{
  /* calculo de visibilidade dos tabs */
  visible_tabs (tabs);

  /* pinta fundo e desenha borda */
  ClearBackground (tabs);
  DrawTabsBorders (tabs);

  /* desenha tabs e decoracoes */
  DrawLineOfTabs (tabs);

  /* termina processo de desenho */
	cdCanvasFlush(tabs->cddbuffer);
}

static int repaint_cb (Ihandle * self)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassName(self), "tabs"))
    return IUP_IGNORE;

  assert (tabs != NULL);
  if (tabs == NULL)
    return IUP_DEFAULT;

  /* verifica se e' um evento do canvas do tabs */
  if (self != tabs->self)
    return IUP_DEFAULT;

  if (!tabs->cddbuffer)
    tabs_createcanvas(tabs);

  assert(tabs->cddbuffer != NULL);
  if (!tabs->cddbuffer)
    return IUP_DEFAULT;

  doRepaint(tabs);

  return IUP_DEFAULT;
}

/*
*  Callback do popup com lista de tabs
*/

static int menu_cb (Ihandle * self)
{
  char *number = iupGetEnv (self, "IUPTABS_ID");
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  int tab_number = 0;
  int result;

  assert (tabs != NULL);
  if (tabs == NULL)
    return IUP_DEFAULT;

  sscanf (number, "%d", &tab_number);

  if (tabs->current_tab != tab_number)
  {
    result = cb_tabchange (tabs, GetTabIhandle (tabs, tab_number),
                                 GetTabIhandle (tabs, tabs->current_tab));

    if (result != IUP_IGNORE)
    {
      tabs->current_tab = tab_number;
      SetNewCurrentTab (tabs);
    }
  }

  return IUP_DEFAULT;
}

/*
* Cria menu com todos os labels dos tabs
*/

static Ihandle *MakeMenuFromTabs (tTabs * tabs)
{
  int c = 0;
  Ihandle *menu = IupMenu (NULL);
  Ihandle *item = NULL;
  char label[5];
  char *temp = NULL;

  for (c = 0; c < tabs->number_of_tabs; c++)
  {
    temp = IupGetAttribute (tabs->tabs_line.tabs_info[c].ihandle, ICTL_TABTITLE);
    if (temp == NULL)
      continue;

    item = IupItem (temp, NULL);
    IupSetCallback(item, "ACTION", (Icallback) menu_cb);

    sprintf (label, "%4d", c);
    IupStoreAttribute (item, "IUPTABS_ID", (char *) label);
    IupSetAttribute (item, "_IUPTABS_DATA", (char *) tabs);

    if (c == tabs->current_tab)
      IupSetAttribute (item, IUP_VALUE, IUP_ON);

    IupAppend (menu, item);
  }

  return menu;
}

static int tabsGetNextTab(tTabs *tabs)
{
  int next_tab;
  if (tabs->current_tab < (tabs->number_of_tabs - 1))
    next_tab = tabs->current_tab + 1;
  else
    next_tab = 0;

  while (next_tab != tabs->current_tab &&
         iupStrEqual(IupGetAttribute(GetTabIhandle(tabs, next_tab), IUP_ACTIVE), IUP_NO))
  {
    if (next_tab < (tabs->number_of_tabs - 1))
      next_tab = next_tab + 1;
    else
      next_tab = 0;
  }

  if (next_tab == tabs->current_tab)
    return -1;

  return next_tab;
}

static int tabsGetPreviousTab(tTabs *tabs)
{
  int previous_tab;
  if (tabs->current_tab > 0)
    previous_tab = tabs->current_tab - 1;
  else
    previous_tab = tabs->number_of_tabs - 1;

  while (previous_tab != tabs->current_tab &&
         iupStrEqual(IupGetAttribute(GetTabIhandle(tabs, previous_tab), IUP_ACTIVE), IUP_NO))
  {
    if (previous_tab > 0)
      previous_tab = previous_tab - 1;
    else
      previous_tab = tabs->number_of_tabs - 1;
  }

  if (previous_tab == tabs->current_tab)
    return -1;

  return previous_tab;
}

/*
* Callback de teclado
*/
static int keypress_cb (Ihandle * self, int c, int press)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassName(self), "tabs"))
    return IUP_DEFAULT;

  if (tabs == NULL || press==0)
    return IUP_DEFAULT;

  if(((c == K_RIGHT||c == K_sRIGHT) && (tabs->tabs_type == TABS_TOP || tabs->tabs_type == TABS_BOTTOM)) ||
     ((c == K_DOWN||c == K_sDOWN) && (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)))
  {
    int result;
    int next_tab = tabsGetNextTab(tabs);
    if (next_tab == -1)
      return IUP_DEFAULT;

    result = cb_tabchange (tabs,
                               GetTabIhandle(tabs, next_tab),
                               GetTabIhandle(tabs, tabs->current_tab));

    if (result != IUP_IGNORE)
    {
      tabs->current_tab = next_tab;  
      SetNewCurrentTab (tabs);
    }
  }
  else if(((c == K_LEFT||c == K_sLEFT) && (tabs->tabs_type == TABS_TOP || tabs->tabs_type == TABS_BOTTOM)) || 
          ((c == K_UP||c == K_sUP) && (tabs->tabs_type == TABS_LEFT || tabs->tabs_type == TABS_RIGHT)))
  {
    int result;
    int previous_tab = tabsGetPreviousTab(tabs);
    if (previous_tab == -1)
      return IUP_DEFAULT;

    result = cb_tabchange (tabs,
                               GetTabIhandle(tabs, previous_tab),
                               GetTabIhandle(tabs, tabs->current_tab));

    if (result != IUP_IGNORE)
    {
      tabs->current_tab = previous_tab;
      SetNewCurrentTab (tabs);
    }
  }

  return IUP_DEFAULT;
}

/*
*  Callback de button press do canvas IUP
*/

static int button_cb (Ihandle * self, int b, int press,
                      int mx, int my)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  static int last_tab_press = -1;
  long width = 0, height = 0, *virtual_width = NULL, *virtual_height = NULL;
  long offset = 0;
  int click_x = mx, click_y = tabs->h-1 - my;
  int *virtual_mx = NULL, *virtual_my = NULL;
  int count = 0;

  /* avoid callback inheritance */
  if (!iupStrEqual(IupGetClassName(self), "tabs"))
    return IUP_IGNORE;

  assert (tabs != NULL);
  if (tabs == NULL)
    return IUP_DEFAULT;

  /* verifica se e' um evento do canvas do tabs */
  if (self != tabs->self)
    return IUP_DEFAULT;

  /* so' responde ao evento do botao esquerdo */
  if (b != IUP_BUTTON1)
    return IUP_DEFAULT;

  /* calcula tamanho da bounding box da fila de tabs */

  switch (tabs->tabs_type)
  {
  case TABS_TOP:
    width = tabs->w;
    height = tabs->tabs_line.line_thick;
    virtual_width = &width;
    virtual_height = &height;
    virtual_mx = &mx;
    virtual_my = &my;
    *virtual_my = -(*virtual_my - (tabs->tabs_line.line_thick - 1));
    break;

  case TABS_BOTTOM:
    width = tabs->w;
    height = tabs->tabs_line.line_thick;
    virtual_width = &width;
    virtual_height = &height;
    virtual_mx = &mx;
    virtual_my = &my;
    *virtual_my = *virtual_my - tabs->h + tabs->tabs_line.line_thick;
    break;

  case TABS_LEFT:
    width = tabs->tabs_line.line_thick;
    height = tabs->h;
    virtual_width = &height;
    virtual_height = &width;
    virtual_mx = &my;
    virtual_my = &mx;
    *virtual_my = -(*virtual_my - (tabs->tabs_line.line_thick - 1));
    break;

  case TABS_RIGHT:
    width = tabs->tabs_line.line_thick;
    height = tabs->h;
    virtual_width = &height;
    virtual_height = &width;
    virtual_mx = &my;
    virtual_my = &mx;
    *virtual_my = *virtual_my - tabs->w + tabs->tabs_line.line_thick;
    break;
  }

  /*
  * ajusta offset para caso de de o primeiro tab nao ser o
  * selecionado
  */

  if (tabs->current_tab != 0)
    offset = 1;

  /* checa se mouse esta' na area dos tabs */
  if ((*virtual_mx > (*virtual_width - 1)) ||
    (*virtual_my > (*virtual_height - 1)) ||
    (*virtual_mx < 0) || (*virtual_my < 0))
  {
    /* se for um button release e algum botao da scrollbar_visible estiver apertado,
    * ele sera' solto */
    if(!press && tabs->tabs_line.button != BUT_NONE )
      tabs->tabs_line.button = BUT_NONE;

    return IUP_DEFAULT;
  }


  if (!press && tabs->tabs_line.button != BUT_NONE)
    tabs->tabs_line.button += 3; /* increment from BUTPRESS_* to BUTRELEASE_* */

  /*
  * se houver scrollbar_visible, verifica se clique foi em um dos botoes
  */

  if (tabs->tabs_line.scrollbar_visible == TABS_TRUE)
  {
    int scroll_pos, scroll_space;

    if (tabs->tabs_orientation == TABS_HORIZONTAL)
    {
      scroll_space = tabs->tabs_line.scroll_w;
      scroll_pos = click_x - tabs->tabs_line.scroll_x;
    }
    else
    {
      scroll_space = tabs->tabs_line.scroll_h;
      scroll_pos = click_y - tabs->tabs_line.scroll_y;

      /* the controls are inverted when text is vertical */
      scroll_pos = scroll_space - scroll_pos;
    }

    if (click_y > tabs->tabs_line.scroll_y && click_x > tabs->tabs_line.scroll_x &&
        click_y < tabs->tabs_line.scroll_y+tabs->tabs_line.scroll_h && click_x < tabs->tabs_line.scroll_x+tabs->tabs_line.scroll_w)
    {
      /* checa em qual dos botoes o usuario clicou */

      if (scroll_pos > 2 * scroll_space / 3)        /* botao de avanco */
      {
        /* se o botao foi apertado, desenha-o pressionado */
        if(press)
        {
          tabs->tabs_line.button = BUTPRESS_FORWARD ;
        }
        else if (tabs->tabs_line.button == BUTRELEASE_FORWARD)
          /* caso contrario, muda o tab */
        {
          int result;
          int next_tab = tabsGetNextTab(tabs);
          if (next_tab == -1)
            return IUP_DEFAULT;

          result = cb_tabchange (tabs,
            GetTabIhandle (tabs, next_tab),
            GetTabIhandle (tabs, tabs->current_tab));

          if (result != IUP_IGNORE)
          {
            tabs->current_tab = next_tab;
            IupSetFocus(tabs->self);
            SetNewCurrentTab(tabs);
          }
        }    
      }                                /* botao menu */
      else if (scroll_pos > scroll_space / 3)
      {
        /* se o botao foi apertado, desenha-o pressionado */
        if(press)
        {
          tabs->tabs_line.button = BUTPRESS_MENU ;
        }
        else if (tabs->tabs_line.button == BUTRELEASE_MENU)
          /* caso contrario, muda o tab */
        {
          Ihandle *menu = MakeMenuFromTabs (tabs);
          IupPopup (menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
          IupDestroy (menu);
        }
      }                                /* botao de recuo */
      else
      {
        /* se o botao foi apertado, desenha-o pressionado */
        if(press)
        {
          tabs->tabs_line.button = BUTPRESS_BACKWARD ;
        }
        else if (tabs->tabs_line.button == BUTRELEASE_BACKWARD)
          /* caso contrario, muda o tab */
        {
          int result;
          int previous_tab = tabsGetPreviousTab(tabs);
          if (previous_tab == -1)
            return IUP_DEFAULT;

          result = cb_tabchange (tabs,
            GetTabIhandle (tabs, previous_tab),
            GetTabIhandle (tabs, tabs->current_tab));

          if (result != IUP_IGNORE)
          {
            tabs->current_tab = previous_tab;
            IupSetFocus(tabs->self);
            SetNewCurrentTab(tabs);
          }
        }
      }

      return IUP_DEFAULT;
    }
  }

  /* se houver algum botao da scrollbar_visible apertado, ele sera solto */
  if(!press && tabs->tabs_line.button != BUT_NONE )
  {
    tabs->tabs_line.button = BUT_NONE;
    IupSetFocus(tabs->self);
    SetNewCurrentTab (tabs);
    return IUP_DEFAULT;
  }

  /* verifica se clique foi na broken tab esquerda */
  if (tabs->tabs_line.broken_start_visible == TABS_TRUE)
  {
    if (*virtual_mx >= offset &&
        *virtual_mx <= (offset + tabs->tabs_line.broken_space_start - 1) &&
        *virtual_my <= (tabs->tabs_line.line_thick - TABS_CURRENT_EXTRA_PIXELS - 1))
    {
      if (press)
        last_tab_press = tabs->tabs_line.start_tab-1;
      else if (last_tab_press == tabs->tabs_line.start_tab-1)
      {
        int result = cb_tabchange (tabs, GetTabIhandle (tabs, last_tab_press), 
                                         GetTabIhandle (tabs, tabs->current_tab));

        if (result != IUP_IGNORE)
        {
          tabs->current_tab = tabs->tabs_line.start_tab - 1;
          IupSetFocus(tabs->self);
          SetNewCurrentTab (tabs);
        }
      }

      return IUP_DEFAULT;
    }
    else
      offset += tabs->tabs_line.broken_space_start;
  }

  /* verifica se clique foi em um dos tabs */
  for (count = tabs->tabs_line.start_tab; count <= tabs->tabs_line.end_tab; count++)
  {
    if (*virtual_mx >= offset && 
        *virtual_mx <= (offset + tabs->tabs_line.tabs_info[count].tab_len - 1))
    {
      if (count != tabs->current_tab)
      {
        if (*virtual_my <=
          (tabs->tabs_line.line_thick - TABS_CURRENT_EXTRA_PIXELS - 1))
        {
          if (press)
            last_tab_press = count;
          else if (last_tab_press == count)
          {
            int result = cb_tabchange (tabs, GetTabIhandle (tabs, last_tab_press),
                                             GetTabIhandle (tabs, tabs->current_tab));

            if (result != IUP_IGNORE)
            {
              tabs->current_tab = count;
              IupSetFocus(tabs->self);
              SetNewCurrentTab (tabs);
            }
          }

          return IUP_DEFAULT;
        }
      }
    }

    offset += tabs->tabs_line.tabs_info[count].tab_len;
  }

  /* verifica se clique foi na broken tab direita */
  if (tabs->tabs_line.broken_end_visible == TABS_TRUE)
  {
    if (*virtual_mx >= offset &&
        *virtual_mx <= (offset + tabs->tabs_line.broken_space_end - 1) &&
        *virtual_my <= (tabs->tabs_line.line_thick - TABS_CURRENT_EXTRA_PIXELS - 1))
    {
      if (press)
        last_tab_press = tabs->tabs_line.end_tab + 1;
      else if (last_tab_press == tabs->tabs_line.end_tab + 1)
      {
        int result = cb_tabchange(tabs, GetTabIhandle (tabs, last_tab_press),
                                        GetTabIhandle (tabs, tabs->current_tab));

        if (result != IUP_IGNORE)
        {
          tabs->current_tab = tabs->tabs_line.end_tab + 1;
          IupSetFocus(tabs->self);
          SetNewCurrentTab (tabs);
        }
      }

      return IUP_DEFAULT;
    }
    else
      offset += tabs->tabs_line.broken_space_end;
  }

  return IUP_DEFAULT;
}

static void tabRefreshTabs(tTabs *tabs)
{
  int t;
  Ihandle* child;

  tabs->number_of_tabs = 0;
  child = IupGetNextChild(tabs->zbox, NULL);
  while(child)
  {
    if (IupGetName(child) == NULL)
    {
      static int tab_count = 0;
      char buffer[30];
      sprintf (buffer, "##$$@TABSNAME%d", tab_count++);
      IupSetHandle(buffer, child);
    }

    tabs->number_of_tabs++;
    child = IupGetBrother(child);
  }

  tabs->tabs_line.tabs_info = (tTabDrawInfo*)realloc(tabs->tabs_line.tabs_info, tabs->number_of_tabs * sizeof (tTabDrawInfo));
  memset(tabs->tabs_line.tabs_info, 0, tabs->number_of_tabs * sizeof (tTabDrawInfo));

  t = 0;
  child = IupGetNextChild(tabs->zbox, NULL); 
  while (child)
  {
    tabs->tabs_line.tabs_info[t].ihandle = child;
    child = IupGetBrother (child);
    t++;
  }
}

/*
*  Metodo iupCpi de criacao do controle TABS
*/

static Ihandle * tabscreate(Iclass *ic, void **params)
{
  Ihandle *self = NULL;
  tTabs *tabs = NULL;
  (void)ic;

  self = IupCanvas(NULL);

  /* aloca memoria para estrutura tTabs */

  tabs = (tTabs *) malloc(sizeof(tTabs));
  memset(tabs, 0, sizeof(tTabs));

  tabs->zbox = IupZbox (NULL);
  tabs->self = self;

  IupAppend(self, tabs->zbox); /* zbox is a child of Tabs */

  /* add children */
  {
    Ihandle *elem = NULL;
    while ((elem = *((Ihandle **) params)) != NULL)
    {
      IupAppend (tabs->zbox, elem);
      params = (void **)(((Ihandle **) params) + 1);
    }
  }

  tabRefreshTabs(tabs);

  /* seta atributos */

  IupSetCallback(self, IUP_ACTION, (Icallback) repaint_cb);
  IupSetCallback(self, IUP_RESIZE_CB, (Icallback) resize_cb);
  IupSetCallback(self, IUP_BUTTON_CB, (Icallback) button_cb);
  IupSetCallback(self, IUP_KEYPRESS_CB, (Icallback) keypress_cb);

  /* inicializa cores */

  tabs->bgcolor = -1;
  tabs->fgcolor = CD_BLACK;

  tabs->light_shadow = CD_WHITE;
  tabs->mid_shadow = CD_GRAY;
  tabs->dark_shadow = CD_DARK_GRAY;

  iupSetEnv (self, "_IUPTABS_DATA", (char *) tabs);

  /* inicializa contexto com valores relativos ao tipo de tabs */  
  tabs->tabs_type = TABS_TOP;
  tabs->tabs_orientation = TABS_HORIZONTAL;
  iupSetEnv(self, "TABORIENTATION", "HORIZONTAL");
  iupSetEnv(self, "TABTYPE", "TOP");

  /* nenhum botao esta pressionado */
  tabs->tabs_line.button = BUT_NONE ;

  iupSetEnv(tabs->self, "_IUPTABS_RESIZE_FIRST_TIME", "1");

  updateRender(tabs);

  return self;
}

static void tabsdestroy (Ihandle * c)
{
  tTabs *tabs = (tTabs *) iupGetEnv (c, "_IUPTABS_DATA");

  assert (tabs);
  if (tabs == NULL)
    return;

  if (tabs->cddbuffer) cdKillCanvas(tabs->cddbuffer);
  if (tabs->cdcanvas) cdKillCanvas(tabs->cdcanvas);

  free (tabs->tabs_line.tabs_info);
  free (tabs);

  iupSetEnv(c, "_IUPTABS_DATA", NULL);
}

static void tabsmap (Ihandle * self, Ihandle * parent)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  assert (tabs != NULL);
  if (tabs == NULL)
    return;

  if (tabs->bgcolor == -1) 
  {
    tabs->bgcolor = cdIupConvertColor(iupGetBgColor(self));
    cdIupCalcShadows(tabs->bgcolor, &tabs->light_shadow, &tabs->mid_shadow, &tabs->dark_shadow);
  }

  iupSetEnv (self, IUP_BORDER, IUP_NO);

  iupCpiDefaultMap (self, parent);

  iupSetEnv (self, IUP_BORDER, NULL);   /* to avoid propagating to the children */

  iupCpiMap (tabs->zbox, self);
}

static void tabssetattr (Ihandle * self, char *attr, char *value)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  tBool do_repaint = TABS_FALSE;        /* indica necessidade de repaint */

  if (tabs == NULL)
  {
    iupCpiDefaultSetAttr (self, attr, value);        /* chama metodo default */
    return;
  }

  if (iupStrEqual (attr, IUP_ALIGNMENT))  /* must set here because ALIGNMENT is not inherited */
  { 
    IupSetAttribute(tabs->zbox, attr, value);
  }
  else if (iupStrEqual (attr, "TABORIENTATION"))
  {
    if (iupStrEqual (value, "VERTICAL"))
      tabs->tabs_orientation = TABS_VERTICAL;
    else
      tabs->tabs_orientation = TABS_HORIZONTAL;

    updateRender(tabs);

    IupRefresh(self);

    do_repaint = TABS_TRUE;
  }
  else if (iupStrEqual (attr, ICTL_TABTYPE))
  {
    if (iupStrEqual (value, ICTL_BOTTOM))
      tabs->tabs_type = TABS_BOTTOM;
    else if (iupStrEqual (value, ICTL_LEFT))
      tabs->tabs_type = TABS_LEFT;
    else if (iupStrEqual (value, ICTL_RIGHT))
      tabs->tabs_type = TABS_RIGHT;
    else
      tabs->tabs_type = TABS_TOP;

    updateRender(tabs);

    IupRefresh(self);

    do_repaint = TABS_TRUE;
  }
  else if (iupStrEqual (attr, IUP_VALUE) && value != NULL)
  {
    Ihandle *new_tab = IupGetHandle (value);
    int c;

    if (new_tab == NULL)
      return;

    for (c = 0; c < tabs->number_of_tabs; c++)
    {
      if (tabs->tabs_line.tabs_info[c].ihandle == new_tab)
      {
        tabs->current_tab = c;
        SetNewCurrentTab (tabs);
        break;
      }
    }
  }
  else if (iupStrEqual (attr, "UPDATE"))
  {
    tabRefreshTabs(tabs);
    calc_tabs_size(tabs);
    calcsize_by_canvas(tabs);
    calc_scroll_pos(tabs);
    do_repaint = TABS_TRUE;
  }
  else if (iupStrEqual(attr, ICTL_FONT_ACTIVE))
  {
    char *native = IupMapFont(value);
    if (!native) native = value;
    if(native)
    {
      if(tabs->fontact)
        free(tabs->fontact);
      tabs->fontact = iupStrDup(native);
    }
  }
  else if (iupStrEqual(attr, ICTL_FONT_INACTIVE))
  {
    char *native = IupMapFont(value);
    if (!native) native = value;
    if(native)
    {
      if(tabs->fontinact)
        free(tabs->fontinact);
      tabs->fontinact = iupStrDup(native);
    }
  }
  else if (iupStrEqual(attr, IUP_BGCOLOR))
  {
    tabs->bgcolor = cdIupConvertColor(value);
    cdIupCalcShadows(tabs->bgcolor, &tabs->light_shadow, &tabs->mid_shadow, &tabs->dark_shadow);
  }
  else if (iupStrEqual(attr, IUP_FGCOLOR))
  {
    tabs->fgcolor = cdIupConvertColor(value);
  }
  else if(iupStrEqual(attr, IUP_REPAINT))
  {
    tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
    if (iupStrEqual(value, IUP_YES))
      SetNewCurrentTab(tabs);
  }
  else
  {
    iupCpiDefaultSetAttr (self, attr, value);        /* chama metodo default */
    return;
  }

  if (tabs->cddbuffer && do_repaint == TABS_TRUE)
    doRepaint(tabs);
}

static char *tabsgetattr (Ihandle * self, char *attr)
{
  tTabs *tabs = (tTabs *) iupGetEnv (self, "_IUPTABS_DATA");
  if (tabs == NULL)
    return iupCpiDefaultGetAttr (self, attr);        /* chama metodo default */

  if (iupStrEqual (attr, IUP_VALUE))
  {
    char *value = iupCpiGetAttribute (tabs->zbox, IUP_VALUE);
    if (value == NULL)
      return iupGetEnv(tabs->zbox, IUP_VALUE);
    else
      return value;
  }
  else
    return iupCpiDefaultGetAttr (self, attr);        /* chama metodo default */
}

/*
*  Criacao do controle
*
*    Recebe lista de Ihandle's dos elementos filhos,
*    terminada por NULL
*/

Ihandle *IupTabsv(Ihandle** params)
{
  return IupCreatev("tabs", params);
}

Ihandle *IupTabs (Ihandle * first,...)
{
  Ihandle **params = NULL;
  Ihandle *elem = NULL;

  va_list arglist;
  va_start(arglist, first);
  params = iupGetParamList(first, arglist);
  va_end(arglist);

  elem = IupCreatev("tabs", params);
  free(params);
  return elem;
}

/*
* Registra controle para uso no LED
*/

void IupTabsOpen (void)
{
  Iclass* ICTabs = iupCpiCreateNewClass("tabs" ,"C");

  iupCpiSetClassMethod(ICTabs, ICPI_SETNATURALSIZE, (Imethod) tabssetnaturalsize);
  iupCpiSetClassMethod(ICTabs, ICPI_SETCURRENTSIZE,(Imethod) tabssetcurrentsize);
  iupCpiSetClassMethod(ICTabs, ICPI_SETPOSITION,(Imethod) tabssetposition);
  iupCpiSetClassMethod(ICTabs, ICPI_CREATE, (Imethod) tabscreate);
  iupCpiSetClassMethod(ICTabs, ICPI_DESTROY, (Imethod) tabsdestroy);
  iupCpiSetClassMethod(ICTabs, ICPI_MAP, (Imethod) tabsmap);
  iupCpiSetClassMethod(ICTabs, ICPI_SETATTR, (Imethod) tabssetattr);
  iupCpiSetClassMethod(ICTabs, ICPI_GETATTR, (Imethod) tabsgetattr);
}
