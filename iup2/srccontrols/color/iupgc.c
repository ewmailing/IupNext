/** \file
 * \brief iupgc pre-defined dialog control
 *
 * See Copyright Notice in iup.h
 * $Id: iupgc.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#include <iup.h>
#include <iupcpi.h>
#include <iupkey.h>
#include <cd.h>
#include <cdiup.h>
#include <iupcb.h>
#include <iupmask.h>

#include "iupgc.h"
#include "icb_rgb.h"
#include "icb_hls.h"
#include "igc_lng.h"
#include "icontrols.h"

#define FSH 50
#define FSW 80 /* (FSH * 1.6) */

#define CONTEXT_ID  12321


/* para debugar/testar o dithering */
/* #define cdGetColorPlanes() (8) */

static long *sys_pal;
static int pal_size;
static int dither;

static uchar shade_r, shade_g, shade_b;
static uchar shade_dr, shade_dg, shade_db;
static uchar shade_lr, shade_lg, shade_lb;

typedef struct _t_context {
  int id;
  int h_down;
  int ls_down;
  float cur_h, cur_l, cur_s;
  uchar cur_r, cur_g, cur_b;
  int h_x, h_y;
  int ls_x, ls_y;
  uchar c_red[FSH*FSW], c_green[FSH*FSW];
  uchar c_blue[FSH*FSW], c_map[FSH*FSW];
  Ihandle *r_txt, *g_txt, *b_txt;
  Ihandle *h_txt, *l_txt, *s_txt;
  Ihandle *c_icv;
  Ihandle *hls_cb;
  cdCanvas *cdcanvas;
  char txt_buf[100];
  int res;
} t_context;

typedef t_context *p_context;

/************************************************************\
* Macros.                                                    *
\************************************************************/
#ifndef max
#define max(a, b)   ((a > b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)   ((a < b) ? (a) : (b))
#endif

/************************************************************\
* Prototipos das funccoes auxiliares.                        *
\************************************************************/
static p_context get_context(Ihandle *s);
static void      put_context(Ihandle *s, p_context con);
static Ihandle  *setattr(Ihandle *s, char *a, char *v);
static Ihandle  *setattrs(Ihandle *s, char *e);
static Ihandle  *create_element(p_context con);

static void hls_gotorgb(p_context con);

static void c_draw(p_context con);
static void c_update(p_context con);
static int  c_repaint(Ihandle *s);
static int  c_resize(Ihandle *s);

static void hls_txt_update(p_context con);
static void rgb_txt_update(p_context con);
static void txt_update(p_context con);
static int  rgb_killfocus(Ihandle *s);
static int  hls_killfocus(Ihandle *s);

static void hls2rgb(p_context con);
static void rgb2hls(p_context con);

static void init_usr_pal(void);
static void init_defaults(p_context con, unsigned char r, unsigned char g, 
  unsigned char b);

/************************************************************\
* Recupera o contexto do controle.                           *
\************************************************************/
static p_context get_context(Ihandle *s)
{
  p_context con;
  con = (p_context) IupGetAttribute(s, "_IUPGC_DATA");
  assert(con != NULL && con->id == CONTEXT_ID);
    
  return con;
}

/************************************************************\
* Pendura o contexto no controle.                            *
\************************************************************/
static void put_context(Ihandle *s, p_context con)
{
  assert(con != NULL);
  IupSetAttribute(s, "_IUPGC_DATA", (char *) con);
}

/************************************************************\
* Converte a cor HLS corrente em RGB.                        *
\************************************************************/
static void hls2rgb(p_context con)
{
  float r, g, b;
  
  hls_hls2rgb(con->cur_h, con->cur_l, con->cur_s, &r, &g, &b);
  r *= 255.0f; g *= 255.0f; b *= 255.0f;
  con->cur_r = touchar(r); con->cur_g = touchar(g); con->cur_b = touchar(b);
}

/************************************************************\
* Converte a cor RGB corrente em HLS.                        *
\************************************************************/
static void rgb2hls(p_context con)
{
  hls_rgb2hls(con->cur_r/255.0f, con->cur_g/255.0f, con->cur_b/255.0f, 
    &(con->cur_h), &(con->cur_l), &(con->cur_s));
}

/************************************************************\
* Redesenha o feedback da cor selecionada.                   *
\************************************************************/
static void c_draw(p_context con)
{
  if (dither) {
    cdCanvasPutImageRectMap(con->cdcanvas, FSW, FSH, con->c_map, sys_pal, 0, 0, FSW, FSH, 0, 0, 0, 0);
  } else {
    cdCanvasPutImageRectRGB(con->cdcanvas, FSW, FSH, con->c_red, con->c_green, con->c_blue, 0, 0, FSW, FSH, 0, 0, 0, 0);
  }
}

/************************************************************\
* Redefine a imagem do feedback de cor selecionada.          *
\************************************************************/
static void c_update(p_context con)
{
  int y;
  long int os;

  memset(con->c_red, con->cur_r, FSW*FSH);
  memset(con->c_green, con->cur_g, FSW*FSH);
  memset(con->c_blue, con->cur_b, FSW*FSH);

  /* desenha a borda */
  memset(con->c_red + FSW*FSH - FSW, shade_dr, FSW);
  memset(con->c_green + FSW*FSH - FSW, shade_dg, FSW);
  memset(con->c_blue + FSW*FSH - FSW, shade_db, FSW);

  os = 0;
  for (y = 0; y < FSH; y++) {
    con->c_red[os] = shade_dr; con->c_green[os] = shade_dg; con->c_blue[os] = shade_db;
    con->c_red[os+FSW-1] = shade_lr; con->c_green[os+FSW-1] = shade_lg; con->c_blue[os+FSW-1] = shade_lb;
    os += FSW;
  }
  
  memset(con->c_red, shade_lr, FSW);
  memset(con->c_green, shade_lg, FSW);
  memset(con->c_blue, shade_lb, FSW);

  if (dither)
    rgb_dither2map(FSW, FSH, con->c_red, con->c_green, con->c_blue, con->c_map);
}

/************************************************************\
* Callback associado ao botao de OK no dilogo de seleccao de *
* cores.                                                     *
\************************************************************/
static int hls_ok(Ihandle *s)
{
  p_context con = get_context(s);
  con->res = 1;
  return IUP_CLOSE;
}

/************************************************************\
* Callback associado ao botao de Cancel no dilogo de sele-   *
* ccao de cores.                                             *
\************************************************************/
static int hls_cancel(Ihandle *s)
{
  p_context con = get_context(s);
  con->res = 0;
  return IUP_CLOSE;
}

/************************************************************\
* Cria os canvas CD que soh podem ser criados apos o map do  *
* canvas...                                                  *
\************************************************************/

static void create_canvas(p_context con)
{
  con->cdcanvas = cdCreateCanvas(CD_IUP, con->c_icv);
}

static int c_resize(Ihandle *s)
{
  p_context con = get_context(s);

  if (!con->cdcanvas) 
    create_canvas(con);

  assert(con->cdcanvas != NULL);
  if (!con->cdcanvas) return IUP_DEFAULT;

  cdCanvasActivate(con->cdcanvas);

  return IUP_DEFAULT;
}

/************************************************************\
* Callback associado de feedback da cor selecionada.         *
\************************************************************/
static int c_repaint(Ihandle *s)
{
  p_context con = get_context(s);

  if (!con->cdcanvas) 
    create_canvas(con);

  assert(con->cdcanvas != NULL);
  if (!con->cdcanvas) return IUP_DEFAULT;
  
  if (cdCanvasGetColorPlanes(con->cdcanvas) < 16) {
    cdCanvasPalette(con->cdcanvas, pal_size, sys_pal, CD_FORCE);
    if (dither != 1) {
      dither = 1;
      c_update(con);
    }
  } else
    dither = 0;
  
  c_draw(con);
  return IUP_DEFAULT;
}

/************************************************************\
* Atualiza os campos de texto com os valores correntes.      *
\************************************************************/
static void hls_txt_update(p_context con)
{
  if (IupGetInt(con->h_txt, IUP_VALUE) != (int) con->cur_h) {
    sprintf(con->txt_buf, "%d", (int) con->cur_h);
    IupSetAttribute(con->h_txt, IUP_VALUE, con->txt_buf);
  }
  if (IupGetFloat(con->l_txt, IUP_VALUE) != con->cur_l) {
    sprintf(con->txt_buf, "%3.2f", con->cur_l);
    IupSetAttribute(con->l_txt, IUP_VALUE, con->txt_buf);
  }
  if (IupGetFloat(con->s_txt, IUP_VALUE) != con->cur_s) {
    sprintf(con->txt_buf, "%3.2f", con->cur_s);
    IupSetAttribute(con->s_txt, IUP_VALUE, con->txt_buf);
  }
}

static void rgb_txt_update(p_context con)
{
  if (IupGetInt(con->r_txt, IUP_VALUE) != (int) con->cur_r) {
    sprintf(con->txt_buf, "%d", (int) con->cur_r);
    IupSetAttribute(con->r_txt, IUP_VALUE, con->txt_buf);
  }
  if (IupGetInt(con->g_txt, IUP_VALUE) != (int) con->cur_g) {
    sprintf(con->txt_buf, "%d", (int) con->cur_g);
    IupSetAttribute(con->g_txt, IUP_VALUE, con->txt_buf);
  }
  if (IupGetInt(con->b_txt, IUP_VALUE) != (int) con->cur_b) {
    sprintf(con->txt_buf, "%d", (int) con->cur_b);
    IupSetAttribute(con->b_txt, IUP_VALUE, con->txt_buf);
  }
}

static void txt_update(p_context con)
{
  hls_txt_update(con);
  rgb_txt_update(con);
}

static void txt_init(p_context con)
{
  IupSetAttribute(con->h_txt, IUP_VALUE, "0");
  IupSetAttribute(con->l_txt, IUP_VALUE, "0.0");
  IupSetAttribute(con->s_txt, IUP_VALUE, "0.0");
  IupSetAttribute(con->r_txt, IUP_VALUE, "0");
  IupSetAttribute(con->g_txt, IUP_VALUE, "0");
  IupSetAttribute(con->b_txt, IUP_VALUE, "0");
}
  
/************************************************************\
* Atualiza o colorbrowser com a cor rgb em con.              *
\************************************************************/
static void hls_gotorgb(p_context con)
{
  sprintf(con->txt_buf, "%3d %3d %3d", con->cur_r, con->cur_g, con->cur_b);
  IupSetAttribute(con->hls_cb, IUP_RGB, con->txt_buf);
}

/************************************************************\
* Callback associado aa mudancca dos valores dos campos de   *
* texto das componentes HLS.                                 *
\************************************************************/
static int hls_killfocus(Ihandle *s) 
{
  p_context con = get_context(s);
  
  con->cur_h = IupGetFloat(con->h_txt, IUP_VALUE);
  con->cur_l = IupGetFloat(con->l_txt, IUP_VALUE);
  con->cur_s = IupGetFloat(con->s_txt, IUP_VALUE);
  hls2rgb(con);
  c_update(con);
  c_repaint(con->c_icv);
  rgb_txt_update(con);
  hls_gotorgb(con);
  
  return IUP_DEFAULT;
}

/************************************************************\
* Callback associado aa mudancca dos valores dos campos de   *
* texto das componentes RGB.                                 *
\************************************************************/
static int rgb_killfocus(Ihandle *s) 
{
  p_context con;

  con = get_context(s);

  con->cur_r = touchar(IupGetInt(con->r_txt, IUP_VALUE));
  con->cur_g = touchar(IupGetInt(con->g_txt, IUP_VALUE));
  con->cur_b = touchar(IupGetInt(con->b_txt, IUP_VALUE));
  rgb2hls(con);
  c_update(con);
  c_repaint(con->c_icv);
  hls_txt_update(con);
  hls_gotorgb(con);

  return IUP_DEFAULT;
}

/************************************************************\
* Define as cores usadas no sombreado do canvas.             *
\************************************************************/
static void init_usr_pal(void)
{
  long int *usr_pal;
  int nc;

  rgb_syspal(&sys_pal, &pal_size);
  rgb_getusrpal(&usr_pal, &nc);

  cdDecodeColor(usr_pal[0], &shade_r, &shade_g, &shade_b);
  cdDecodeColor(usr_pal[1], &shade_lr, &shade_lg, &shade_lb);
  cdDecodeColor(usr_pal[2], &shade_dr, &shade_dg, &shade_db);
}

/************************************************************\
* Inicializa os valores iniciais para o elemento.            *
\************************************************************/
static void init_defaults(p_context con, unsigned char r, unsigned char g, 
  unsigned char b)
{
  char buffer[20];
  
  sprintf(buffer, "%3d %3d %3d", (int) r, (int) g, (int) b);
  IupSetAttribute(con->hls_cb, IUP_RGB, buffer);
  txt_init(con);
  con->cur_r = r;
  con->cur_g = g;
  con->cur_b = b;
  rgb2hls(con);
  txt_update(con);
  c_update(con);
  con->res = 0;
}

/************************************************************\
* Callback de mudancca na cor do colorbrowser.               *
\************************************************************/
static int hls_drag_cb(Ihandle *s, unsigned char r, unsigned char g, 
  unsigned char b)
{
  p_context con = get_context(s);
  
  con->cur_r = r;
  con->cur_g = g;
  con->cur_b = b;
  rgb2hls(con);
  rgb_txt_update(con);
  hls_txt_update(con);
  c_update(con);
  c_repaint(con->c_icv);

  return IUP_DEFAULT;
}

/************************************************************\
* Inicializa a biblioteca.                                   *
\************************************************************/
void IupGetColorOpen(void)
{
  init_usr_pal();
}


/************************************************************\
* Mostra a caixa de dialogos para seleccao de cores.         *
\************************************************************/
int IupGetColor(int x, int y, unsigned char *r, unsigned char *g, 
  unsigned char *b)
{
  Ihandle *icb = NULL;
  int ret = 0;
  p_context con = NULL;
  Ihandle *s;

  con = (p_context) calloc(1, sizeof(t_context));
  assert(con != NULL);
  con->id = CONTEXT_ID;

  s = create_element(con);
  put_context(s, con);

  icb = IupDialog(s);
  IupSetAttribute(icb, IUP_TITLE, iupgcGetText("colorsel"));
  IupSetAttribute(icb, IUP_RESIZE, IUP_NO);
  IupSetAttribute(icb, IUP_MAXBOX, IUP_NO);
  IupSetAttribute(icb, IUP_DEFAULTENTER, "IupGetColorOkButton");
  IupSetAttribute(icb, IUP_DEFAULTESC,   "IupGetColorCancelButton");
  IupSetAttribute(icb, IUP_PARENTDIALOG, IupGetGlobal(IUP_PARENTDIALOG));
  IupSetAttribute(icb, IUP_ICON, IupGetGlobal(IUP_ICON));
  
  IupMap(icb);
  
  init_defaults(con, *r, *g, *b);
    
  IupPopup(icb, x, y);

  if (con->res) {
    *r = (unsigned char) con->cur_r;
    *g = (unsigned char) con->cur_g;
    *b = (unsigned char) con->cur_b;
  }

  ret = con->res;
  cdKillCanvas(con->cdcanvas);

  iupmaskRemove(con->r_txt);
  iupmaskRemove(con->g_txt);
  iupmaskRemove(con->b_txt);
  iupmaskRemove(con->h_txt);
  iupmaskRemove(con->l_txt);
  iupmaskRemove(con->s_txt);

  IupDestroy(icb);
  free(con);

  return ret;
}

/************************************************************\
* Seta um atributo e retorna o elemento.                     *
\************************************************************/
static Ihandle *setattr(Ihandle *s, char *a, char *v)
{
  IupSetAttribute(s, a, v);
  return s;
}

static Ihandle *setattrs(Ihandle *s, char *e)
{
  IupSetAttributes(s, e);
  return s;
}

/************************************************************\
* Retorna um Ihandle para a box mais externa do dialogo.     *
\************************************************************/
static Ihandle *create_element(p_context con)
{
  Ihandle *rgb_vb, *hls_vb, *element, *ok_bt, *cancel_bt;
  char attr_buf[300];
  

  ok_bt = setattr(IupButton(iupgcGetText("ok"), NULL), IUP_SIZE, "45");
  IupSetCallback(ok_bt, "ACTION", (Icallback)hls_ok);
  IupSetHandle( "IupGetColorOkButton", ok_bt );

  cancel_bt = setattr(IupButton(iupgcGetText("cancel"), NULL), IUP_SIZE, "45");
  IupSetCallback(cancel_bt, "ACTION", (Icallback)hls_cancel);
  IupSetHandle( "IupGetColorCancelButton", cancel_bt );

  con->hls_cb = IupColorBrowser();
  IupSetCallback(con->hls_cb, "DRAG_CB", (Icallback) hls_drag_cb);
  IupSetCallback(con->hls_cb, "CHANGE_CB", (Icallback) hls_drag_cb);

  con->c_icv = IupCanvas(NULL);
  sprintf(attr_buf, "RASTERSIZE=%dx%d,"
    "BORDER = NO,"
    "EXPAND = NO,",
    FSW, FSH
  );
  IupSetAttributes(con->c_icv, attr_buf);
  IupSetCallback(con->c_icv, "ACTION", (Icallback)c_repaint);
  IupSetCallback(con->c_icv, "RESIZE_CB", (Icallback)c_resize);

  con->r_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->r_txt, "KILLFOCUS_CB", (Icallback)rgb_killfocus);
  iupmaskSet(con->r_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  con->g_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->g_txt, "KILLFOCUS_CB", (Icallback)rgb_killfocus);
  iupmaskSet(con->g_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  con->b_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->b_txt, "KILLFOCUS_CB", (Icallback)rgb_killfocus);
  iupmaskSet(con->b_txt, "/d|/d/d|1/d/d|2(0|1|2|3|4)/d|25(0|1|2|3|4|5)", 0, 0);

  con->h_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->h_txt, "KILLFOCUS_CB", (Icallback)hls_killfocus);
  iupmaskSet(con->h_txt, "/d|/d/d|(1|2)/d/d|3(0|1|2|3|4|5)/d", 0, 0);

  con->l_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->l_txt, "KILLFOCUS_CB", (Icallback)hls_killfocus);
  iupmaskSet(con->l_txt, "0|(0./d)|(0./d/d)|1", 0, 0);

  con->s_txt = setattrs(IupText(NULL), 
    "SIZE = 34x12"
  );
  IupSetCallback(con->s_txt, "KILLFOCUS_CB", (Icallback)hls_killfocus);
  iupmaskSet(con->s_txt, "0|(0./d)|(0./d/d)|1", 0, 0);

  rgb_vb = IupVbox
  (
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("red")), IUP_SIZE, "28"),
    con->r_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   setattr(IupFill(), IUP_SIZE, "2"),
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("green")), IUP_SIZE, "28"),
    con->g_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   setattr(IupFill(), IUP_SIZE, "2"),
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("blue")), IUP_SIZE, "28"),
    con->b_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   NULL
  );
  
  hls_vb = IupVbox
  (
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("hue")), IUP_SIZE, "28"),
    con->h_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   setattr(IupFill(), IUP_SIZE, "2"),
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("lum")), IUP_SIZE, "28"),
    con->l_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   setattr(IupFill(), IUP_SIZE, "2"),
   setattr(IupHbox 
   (
    setattr(IupLabel(iupgcGetText("sat")), IUP_SIZE, "28"),
    con->s_txt,
    NULL
   ), IUP_ALIGNMENT, IUP_ACENTER), 
   NULL
  );

  element = IupVbox
  (
   setattr(IupFill(), IUP_SIZE, "5"),
   IupHbox
   (
    setattr(IupFill(), IUP_SIZE, "5"),
    con->hls_cb,
    setattr(IupFill(), IUP_SIZE, "10"),
    IupVbox
    (
     IupHbox
     (
      rgb_vb,
      setattr(IupFill(), IUP_SIZE, "8"),
      hls_vb,
      setattr(IupFill(), IUP_SIZE, "5"),
      NULL
     ),
     IupFill(),
     IupHbox
     (
      IupFill(),
      con->c_icv,
      IupFill(),
      NULL
     ),
     IupFill(),
     IupHbox
     (
      IupFill(),
      ok_bt,
      setattr(IupFill(), IUP_SIZE, "5"),
      cancel_bt,
      IupFill(),
      NULL
     ),
     NULL
    ),
    NULL
   ),
   setattr(IupFill(), IUP_SIZE, "5"),
   NULL
  );
    
  return element;
}
