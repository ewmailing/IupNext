/** \file
 * \brief iupcb control
 *
 * See Copyright Notice in iup.h
 *  */

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

#include <iupcompat.h>
#include "iupcb.h"
#include "icontrols.h"

#include "icb_rgb.h"
#include "icb_hls.h"

#define BD 180
#define MS 6
#define DT 4
#define H (BD+1)
#define CR (BD)/2
#define SD (BD - 2*CL)
#define CL 18
#define TS ((SD*4)/5 - 2*DT + 1)
#define LS TS
static const float pi_180 = 180.0f/3.1415926f;

#define CONTEXT_ID 1771

typedef void (*p_drag_cb) (Ihandle *self, uchar r, uchar g, uchar b);
typedef void (*p_change_cb) (Ihandle *self, uchar r, uchar g, uchar b);

/* para testar o dithering */
/* #define cdGetColorPlanes() (8) */

static uchar h_red[H*H], h_green[H*H];
static uchar h_blue[H*H], h_map[H*H];
static long *sys_pal;
static int pal_size;

static long int usr_pal[4];
static uchar shade_r, shade_g, shade_b;
static uchar shade_lr, shade_lg, shade_lb;
static uchar shade_dr, shade_dg, shade_db;
static int dither;

typedef struct _t_context {
  int id;
  int h_down;
  int ls_down;
  float cur_h, cur_l, cur_s;
  uchar cur_r, cur_g, cur_b;
  int h_x, h_y;
  int ls_x, ls_y;
  uchar ls_red[LS*LS], ls_green[LS*LS];
  uchar ls_blue[LS*LS], ls_map[LS*LS];
  Ihandle *hls_icv;
  cdCanvas *cdcanvas;
  char txt_buf[100];
  int res;
} t_context;

typedef t_context *p_context;

/************************************************************\
* Macros.                                                    *
\************************************************************/
#define sq(x)       ((x)*(x))
#ifndef max
#define max(a, b)   ((a > b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)   ((a < b) ? (a) : (b))
#endif
#define in_h(x, y)  (  ( ( rd = (sq(x-CR) + sq(y-CR)) ) > sq(CR-CL) ) && (rd < sq(CR))  )
#define in_ls(x, y) (  (x > (CL+SD/5)) && (x  < 2*(BD-y)-CL) && (x < 2*y-CL+DT) )

/************************************************************\
* Prototipos das funccoes auxiliares.                        *
\************************************************************/
static void cb_destroy(Ihandle* self);
static Ihandle *cb_create(Iclass *ic);
static char    *cb_getattr(Ihandle* self, char* attr);
static void    cb_setattr(Ihandle* self, char* attr, char* value);

static p_context get_context(Ihandle *s);
static void      put_context(Ihandle *s, p_context con);
static Ihandle  *create_element(p_context con);

static int  hls_button(Ihandle *s, int b, int e, int x, int y);
static int  hls_motion(Ihandle *s, int x, int y);
static int  hls_repaint(Ihandle *s) ;
static int  hls_keypress(Ihandle *s, int c, int press);
static int  hls_resize(Ihandle *s);

static void h_draw(p_context con);
static void h_mark(p_context con, int x, int y);
static void h_unmark(p_context con);
static int  h_button(p_context con, int e, int x, int y);
static int  h_motion(p_context con, int x, int y);

static void ls_update(p_context con);
static void ls_draw(p_context con);
static void ls_repaint(p_context con);
static void ls_mark(p_context con, int x, int y);
static void ls_unmark(p_context con);
static int  ls_button(p_context con, int e, int x, int y);
static int  ls_motion(p_context con, int x, int y);

static void adjust_h_xy(p_context con, int *x, int *y);
static void adjust_ls_xy(p_context con, int *x, int *y);

static void hls2rgb(p_context con);
static void rgb2hls(p_context con);
static void hls2xy(p_context con);

static void init_usr_pal(void);
static void init_ls_plasma(p_context con);
static void init_h_plasma(void);
static void init_defaults(p_context con, unsigned char r, 
  unsigned char g, unsigned char b);

/************************************************************\
* Recupera o contexto do controle.                           *
\************************************************************/
static p_context get_context(Ihandle *s)
{
  p_context con;
  con = (p_context) iupGetEnv(s, "_IUPCB_DATA");

  assert(con != NULL && con->id == CONTEXT_ID);
  return con;
}

/************************************************************\
* Pendura o contexto no controle.                            *
\************************************************************/
static void put_context(Ihandle *s, p_context con)
{
  iupSetEnv(s, "_IUPCB_DATA", (char *) con);
}

/************************************************************\
* Desenha a marca de feedback no circulo de H.               *
\************************************************************/
static void h_mark(p_context con, int x, int y)
{
  cdCanvasSector(con->cdcanvas, x, y, MS, MS, 0, 360);
  con->h_x = x; con->h_y = y;
}

static void h_unmark(p_context con)
{
  cdCanvasSector(con->cdcanvas, con->h_x, con->h_y, MS, MS, 0, 360);
}

/************************************************************\
* Desenha a marca de feedback no triangulo de LS.            *
\************************************************************/
static void ls_mark(p_context con, int x, int y)
{
  cdCanvasSector(con->cdcanvas, x, y, MS, MS, 0, 360);
  con->ls_x = x; con->ls_y = y;
}

static void ls_unmark(p_context con)
{
  cdCanvasSector(con->cdcanvas, con->ls_x, con->ls_y, MS, MS, 0, 360);
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
* Redesenha o circulo de H.                                  *
\************************************************************/
static void h_draw(p_context con)
{
  cdCanvasWriteMode(con->cdcanvas, CD_REPLACE);
  if (dither) {
    cdCanvasPutImageRectMap(con->cdcanvas, H, H, h_map, sys_pal, 0, 0, H, H, 0, 0, 0, 0);
  } else {
    cdCanvasPutImageRectRGB(con->cdcanvas, H, H, h_red, h_green, h_blue, 0, 0, H, H, 0, 0, 0, 0);
  }
  cdCanvasWriteMode(con->cdcanvas, CD_NOT_XOR);
  h_unmark(con);
  ls_unmark(con);
}

/************************************************************\
* Atualiza a imagem de LS em funccao de um novo H.           *
\************************************************************/
static void ls_update(p_context con)
{
  hls_lsplasma(LS, LS, con->cur_h, con->ls_red, con->ls_green, con->ls_blue);
  if (dither)
    rgb_dither2map(LS, LS, con->ls_red, con->ls_green, con->ls_blue, con->ls_map);
}

/************************************************************\
* Redesenha o triangulo de LS.                               *
\************************************************************/
static void ls_draw(p_context con)
{
  ls_unmark(con);
  h_unmark(con);
  cdCanvasWriteMode(con->cdcanvas, CD_REPLACE);
  if (dither) {
    cdCanvasPutImageRectMap(con->cdcanvas, LS, LS, con->ls_map, sys_pal, SD/5+CL+DT, SD/10+CL+DT, LS, LS, 0, 0, 0, 0);
  } else {
    cdCanvasPutImageRectRGB(con->cdcanvas, LS, LS, con->ls_red, con->ls_green, con->ls_blue, SD/5+CL+DT, SD/10+CL+DT, LS, LS, 0, 0, 0, 0);
  }
  cdCanvasWriteMode(con->cdcanvas, CD_NOT_XOR);
  h_unmark(con);
  ls_unmark(con);
}

/************************************************************\
* Redesenha o triangulo de LS.                               *
\************************************************************/
static void ls_repaint(p_context con)
{
  if (con->cdcanvas != NULL) {
    ls_draw(con);
  }
}

/************************************************************\
* Chama o callback do usuario avisando da mudancca de cor.   *
\************************************************************/
static void rgb_change(p_context con)
{
  p_change_cb change;

  change = (p_change_cb) IupGetCallback(con->hls_icv, IUP_CHANGE_CB);
  if (change != NULL)
    change(con->hls_icv, con->cur_r, con->cur_g, con->cur_b);
}
  
static void rgb_drag(p_context con)
{
  p_drag_cb drag;

  drag = (p_change_cb) IupGetCallback(con->hls_icv, IUP_DRAG_CB);
  if (drag != NULL)
    drag(con->hls_icv, con->cur_r, con->cur_g, con->cur_b);
}

/************************************************************\
* Determina as coordenadas do canvas correspondentes aos va- *
* lores HLS correntes.                                       *
\************************************************************/
static void hls2xy(p_context con)
{
  con->h_x = (int) (BD/2 + (SD/2 + CL/2) * cos(con->cur_h/pi_180));
  con->h_y = (int) (BD/2 + (SD/2 + CL/2) * sin(con->cur_h/pi_180));

  con->ls_y = (int) ( CL + SD/10 + DT + con->cur_l * TS );
  con->ls_x = (int) ( CL + SD/5 + DT + con->cur_s * (TS - abs(2 * (con->ls_y - CL - DT) - SD/5 - TS)));
}

/************************************************************\
* Ajusta as coordenadas para o centro do circulo de H e cal- *
* cula as componentes HLS alteradas.                         *
\************************************************************/
static void adjust_h_xy(p_context con, int *x, int *y)
{
  float c;
  
  *x -= BD/2;
  *y -= BD/2;
  c = (float) sq(SD/2 + CL/2);
  c /= sq(*x) + sq(*y);
  c = (float) sqrt(c);
  *x = (int) (*x * c + (*x < 0 ? -0.5 : 0.5) );
  *y = (int) (*y * c + (*x < 0 ? -0.5 : 0.5) );
  *x += BD/2;
  *y += BD/2;

  con->cur_h = (float) (atan2(*y - BD/2, *x - BD/2) * pi_180);
  con->cur_h = (float) fmod(con->cur_h, 360.0f);
  if (con->cur_h < 0.0f)
    con->cur_h += 360.0f;
}

/************************************************************\
* Ajusta as coordenadas para o centro do triangulo de LS e   *
* calcula as componentes HLS alteradas.                      *
\************************************************************/
static void adjust_ls_xy(p_context con, int *x, int *y)
{
  int d;
  
  *x = max(*x, CL + SD/5 + DT);
  *x = min(*x, CL + SD/5 + DT + TS);
  *y = max(*y, *x/2 + CL/2 + DT/2);
  *y = min(*y, BD - *x/2 - CL/2 - DT/2);

  d = TS - abs(2 * (*y - CL - DT) - SD/5 - TS);
  if (d != 0)
    con->cur_s = (float) (*x - CL - SD/5 - DT) / (float) (d);
  else
    con->cur_s = 1.0f;
  con->cur_s = max(min(con->cur_s, 1.0f), 0.0f);
  con->cur_l = (float) (*y - CL - SD/10 - DT) / (float) (TS);
  con->cur_l = max(min(con->cur_l, 1.0f), 0.0f);
}

/************************************************************\
* Callback associado ao botao do mouse no circulo de H.      *
\************************************************************/
static int h_button(p_context con, int e, int x, int y)
{
  if (!e) {
    h_unmark(con);
    adjust_h_xy(con, &x, &y);
    h_mark(con, x, y);
    hls2rgb(con);

    rgb_change(con);

    ls_update(con);
    ls_draw(con);
  }
  return IUP_DEFAULT;
}

/************************************************************\
* Callback ao movimento do mouse no circulo de H.            *
\************************************************************/
static int h_motion(p_context con, int x, int y)
{
  h_unmark(con);
  adjust_h_xy(con, &x, &y);
  h_mark(con, x, y);
  hls2rgb(con);

  rgb_drag(con);

  ls_update(con);
  ls_draw(con);
  return IUP_DEFAULT;
}

/************************************************************\
* Callback associado ao botao do mouse no triangulo de LS.   *
\************************************************************/
static int ls_button(p_context con, int e, int x, int y)
{
  if (!e) {
    ls_unmark(con);
    adjust_ls_xy(con, &x, &y);
    ls_mark(con, x, y);
    hls2rgb(con);
    rgb_change(con);
  }
  return IUP_DEFAULT;
}

/************************************************************\
* Callback ao movimento do mouse no triangulo de LS.         *
\************************************************************/
static int ls_motion(p_context con, int x, int y)
{
  ls_unmark(con);
  adjust_ls_xy(con, &x, &y);
  ls_mark(con, x, y);
  hls2rgb(con);
  rgb_drag(con);

  return IUP_DEFAULT;
}

/************************************************************\
* Callback ao botao do mouse no canvas.                      *
\************************************************************/
static int hls_button(Ihandle *s, int b, int e, int x, int y)
{
  int rd;
  p_context con;

  if (b != IUP_BUTTON1)
    return IUP_DEFAULT;

  con = get_context(s);

  cdCanvasUpdateYAxis(con->cdcanvas, &y);

  /* casos esperados */
  if (!con->h_down && e && in_h(x, y)) {
    cdCanvasWriteMode(con->cdcanvas, CD_NOT_XOR);
    h_button(con, 1, x, y);
    con->h_down = 1;
  } 
  if (con->h_down && !e) {
    h_button(con, 0, x, y);
    con->h_down = 0;
  }
  if (!con->ls_down && e && in_ls(x, y)) {
    cdCanvasWriteMode(con->cdcanvas, CD_NOT_XOR);
    ls_button(con, 1, x, y);
    con->ls_down = 1;
  } 
  if (con->ls_down && !e) {
    ls_button(con, 0, x, y);
    con->ls_down = 0;
  }

  return IUP_DEFAULT;
}

/************************************************************\
* Callback associado ao movimento do monse no canvas.        *
\************************************************************/
static int hls_motion(Ihandle *s, int x, int y)
{
  p_context con;

  con = get_context(s);
  
  if (con->h_down) {
    cdCanvasUpdateYAxis(con->cdcanvas, &y);
    h_motion(con, x, y);
  } else if (con->ls_down) {
    cdCanvasUpdateYAxis(con->cdcanvas, &y);
    ls_motion(con, x, y);
  }

  return IUP_DEFAULT;
}

/************************************************************\
* Cria os canvas CD que soh podem ser criados apos o map do  *
* canvas...                                                  *
\************************************************************/
static void create_canvas(p_context con)
{
  con->cdcanvas = cdCreateCanvas(CD_IUP, con->hls_icv);
}

static int hls_resize(Ihandle *s)
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
* Callback associado ao redesenho do canvas HLS.             *
\************************************************************/
static int hls_repaint(Ihandle *s) 
{
  p_context con = get_context(s);

  if (!con->cdcanvas) 
    create_canvas(con);

  assert(con->cdcanvas != NULL);
  if (!con->cdcanvas) return IUP_DEFAULT;

  if (cdCanvasGetColorPlanes(con->cdcanvas) < 16) {
    cdCanvasPalette(con->cdcanvas, pal_size, sys_pal, CD_FORCE);
    if (!dither) {
      rgb_dither2map(LS, LS, con->ls_red, con->ls_green, con->ls_blue, 
        con->ls_map);
    }
    dither = 1;
  } else {
    dither = 0;
  }

  cdCanvasBackground(con->cdcanvas, usr_pal[0]);
  cdCanvasClear(con->cdcanvas);

  h_draw(con);
  ls_draw(con);

  return IUP_DEFAULT;
}

/************************************************************\
* Callback associado aas setas no canvas LS.                 *
\************************************************************/
static int hls_keypress(Ihandle *s, int c, int press)
{
  int x, y;
  p_context con;

  if (!press)
    return IUP_DEFAULT;

  con = get_context(s);

  x = con->ls_x;
  y = con->ls_y;
  
  switch (c) {
  case K_UP:
    y++; break;
  case K_DOWN:
    y--; break;
  case K_RIGHT:
    x++; break;
  case K_LEFT:
    x--; break;
  default:
    return IUP_DEFAULT;
  }

  ls_unmark(con);
  adjust_ls_xy(con, &x, &y);
  ls_mark(con, x, y);
  hls2rgb(con);
  rgb_change(con);

  return IUP_DEFAULT;
}

/************************************************************\
* Define as cores usadas no sombreado do canvas.             *
\************************************************************/
static void init_usr_pal(void)
{
  unsigned int ur, ug, ub;
  int r, g, b;

  rgb_depth(125);

  rgb_syspal(&sys_pal, &pal_size);

  iupGetRGB( IupGetGlobal("DLGBGCOLOR"), &ur, &ug, &ub);

  r = (int) ur;
  g = (int) ug;
  b = (int) ub;

  dither = atoi(IupGetGlobal("SCREENDEPTH")) < 16; 
  
  shade_r = touchar(r);
  shade_g = touchar(g);
  shade_b = touchar(b);

  usr_pal[0] = cdEncodeColor((uchar)r, (uchar)g, (uchar)b); 
  shade_dr = (gcuchar)((2*r)/3); shade_dg = (gcuchar)((2*g)/3); shade_db = (gcuchar)((2*b)/3);
  shade_lr = (gcuchar)((255+r)/2); shade_lg = (gcuchar)((255+g)/2); shade_lb = (gcuchar)((255+b)/2);
  
  hls_shade(shade_r, shade_g, shade_b, shade_lr, shade_lg, 
    shade_lb, shade_dr, shade_dg, shade_db);
  usr_pal[1] = cdEncodeColor(shade_lr, shade_lg, shade_lb);
  usr_pal[2] = cdEncodeColor(shade_dr, shade_dg, shade_db);
  rgb_setusrpal(usr_pal, 3);
}

/************************************************************\
* Inicializa a imagem de LS pegando parte da imagem H.       *
\************************************************************/
static void init_ls_plasma(p_context con)
{
  int i;
  long int hos, lsos;

  hos = (SD/10 + CL + DT)*H + SD/5 + CL + DT;
  lsos = 0;
  for (i = 0; i < LS; i++) {
    memcpy(con->ls_red + lsos, h_red + hos, LS);
    memcpy(con->ls_green + lsos, h_green + hos, LS);
    memcpy(con->ls_blue + lsos, h_blue + hos, LS);
    lsos += LS;
    hos += H;
  }

  hls_lsplasma(LS, LS, con->cur_h, con->ls_red, con->ls_green, con->ls_blue);
  if (dither)
    rgb_dither2map(LS, LS, con->ls_red, con->ls_green, con->ls_blue, con->ls_map);
}

/************************************************************\
* Inicializa a imagem de H.                                  *
\************************************************************/
static void init_h_plasma(void)
{
  hls_hplasma(H, H, CL, h_red, h_green, h_blue);
  rgb_dither2map(H, H, h_red, h_green, h_blue, h_map);
}

/************************************************************\
* Inicializa os valores iniciais para o elemento.            *
\************************************************************/
static void init_defaults(p_context con, unsigned char r, 
  unsigned char g, unsigned char b)
{
  con->cur_r = r;
  con->cur_g = g;
  con->cur_b = b;
  rgb2hls(con);
  hls2xy(con);
  init_ls_plasma(con);
  con->h_down = 0;
  con->ls_down = 0;
}

/************************************************************\
* Inicializa a biblioteca.                                   *
\************************************************************/
void IupColorBrowserOpen(void)
{
  Iclass* ICColorBrowser = iupCpiCreateNewClass("colorbrowser", NULL);
  iupCpiSetClassMethod(ICColorBrowser, ICPI_CREATE, (Imethod) cb_create);
  iupCpiSetClassMethod(ICColorBrowser, ICPI_GETATTR, (Imethod) cb_getattr);
  iupCpiSetClassMethod(ICColorBrowser, ICPI_SETATTR, (Imethod) cb_setattr);
  iupCpiSetClassMethod(ICColorBrowser, ICPI_DESTROY, (Imethod) cb_destroy);

  init_usr_pal();
  init_h_plasma();
}

/************************************************************\
* Cria o elemento CPI.                                       *
\************************************************************/
static Ihandle *cb_create(Iclass *ic)
{
  Ihandle *s;
  p_context con;
  (void)ic;

  con = (p_context) calloc(1, sizeof(t_context));
  assert(con != NULL);
  if (!con)
    return NULL;
  con->id = CONTEXT_ID;

  s = create_element(con);
  put_context(s, con);
  init_defaults(con, 255, 0, 0);

  return s;
}

/***************************************************************************\
* Recupera attributos do elemento ou responde a querys possiveis.           *
\***************************************************************************/
static char *cb_getattr(Ihandle* self, char* attr)
{
  static char rgb_buf[20];
  p_context con = (p_context) get_context(self);

  assert(con != NULL);
  if (con == NULL)
    return iupCpiDefaultGetAttr(self, attr);

  if (iupStrEqual(attr, IUP_RGB)) {
    sprintf(rgb_buf, "%d %d %d", (int) con->cur_r, (int) con->cur_g, (int) con->cur_b);
    return rgb_buf;
  } else {
    return iupCpiDefaultGetAttr(self, attr);
  }
}

/***************************************************************************\
* Libera a memoria alocada pelo controle.                                   *
\***************************************************************************/
static void cb_destroy(Ihandle* self)
{
  p_context con = (p_context) get_context(self);
  if (!con)
    return;
  if (con->cdcanvas)
    cdKillCanvas(con->cdcanvas);
  free(con);
  put_context(self, NULL);
}

/***************************************************************************\
* Altera os atributos do elemento ou realiza as acoes possiveis.            *
\***************************************************************************/
static void cb_setattr(Ihandle *self, char *attr, char *val)
{
  p_context con = (p_context) get_context(self);
  int r, g, b;

  assert(con != NULL);
  if (con == NULL) {
    iupCpiDefaultSetAttr(self, attr, val);
    return;
  }

  if (iupStrEqual(attr, IUP_RGB)) {
    if (sscanf(val, "%d %d %d", &r, &g, &b) != 3)
      return;
    con->cur_r = touchar(r);
    con->cur_g = touchar(g);
    con->cur_b = touchar(b);
    rgb2hls(con);
    ls_update(con);
    if (con->cdcanvas != NULL) {
      ls_repaint(con);
      ls_unmark(con); h_unmark(con);
    }
    hls2xy(con);
    if (con->cdcanvas != NULL) {
      ls_unmark(con); h_unmark(con);
    }
  } else {
    iupCpiDefaultSetAttr(self, attr, val);
  }

}

/***************************************************************************\
* Cria o elemento ColorBrowser.                                             *
\***************************************************************************/
Ihandle *IupColorBrowser(void)
{
  return IupCreate("colorbrowser");
}

/************************************************************\
* Retorna um Ihandle para a box mais externa do dialogo.     *
\************************************************************/
static Ihandle *create_element(p_context con)
{
  char attr_buf[300];
  con->hls_icv = IupCanvas(NULL);
  sprintf(attr_buf, 
    "RASTERSIZE=%dx%d,\n"
    "BORDER = NO,\n"
    "EXPAND = NO",
    H, H
  );
  IupSetCallback(con->hls_icv, "ACTION", (Icallback) hls_repaint);
  IupSetCallback(con->hls_icv, "RESIZE_CB", (Icallback) hls_resize);
  IupSetCallback(con->hls_icv, "BUTTON_CB", (Icallback) hls_button);
  IupSetCallback(con->hls_icv, "MOTION_CB", (Icallback) hls_motion);
  IupSetCallback(con->hls_icv, "KEYPRESS_CB", (Icallback) hls_keypress);

  IupSetAttributes(con->hls_icv, attr_buf);
  sprintf(attr_buf, "%d %d %d", (int) shade_r, (int) shade_g, (int) shade_b);
  IupStoreAttribute(con->hls_icv, IUP_BGCOLOR, "192 192 192");
  return con->hls_icv;
}
