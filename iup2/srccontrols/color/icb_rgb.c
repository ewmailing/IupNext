/** \file
 * \brief iupcb control
 * error difusion dither with a fixed palette of 216 colors.
 *
 * See Copyright Notice in iup.h
 * $Id: icb_rgb.c,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "icb_rgb.h"
#include "icb_tabs.h"

/************************************************************\
* Palheta do usuario.                                        *
\************************************************************/
static uchar usr_r[4];
static uchar usr_g[4];
static uchar usr_b[4];
static int usr_n = 0;

static uchar *trc = trc6;
static uchar *rb = rb6; 
static uchar *gb = gb6;
static uchar *bb = bb6;
static long int *pal = pal6;

static int depth = 216;

/************************************************************\
* Troca o valor de dois ponteiros.                           *
\************************************************************/
static void swap(int **a, int **b)
{
  int *tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

/************************************************************\
* Faz dithering error difusion em um canal da imagem.        *
\************************************************************/
static void difusioninplace(int w, int h, uchar *buf)
{
  int x, y, k;
  int err, srr, drr;
  int *cur, *nxt;
  long int cos, nos;
  
  /* aloca espacco para duas linhas de trabalho */
  cur = (int *) malloc(w * sizeof(int));
  nxt = (int *) malloc(w * sizeof(int));

  nos = w;
  cos = 0;

  /* inicializa o buffer de proxima linha */
  for (x = 0; x < w; x++)
    nxt[x] = buf[x];

  for (y = 0; y < h-1; y++) {

    /* a proxima linha vira a linha corrente */
    swap(&cur, &nxt);

    /* pega a nova proxima linha */
    for (x = 0; x < w; x++)
      nxt[x] = buf[nos + x];

    for (x = 1; x < w-1; x++) {
      k = trc[touchar(cur[x])];
      buf[cos + x] = k;
      srr = cur[x] - k;
      drr = srr >> 3;
      err = srr >> 4;
      nxt[x + 1] += err; srr -= err; err += drr;
      nxt[x - 1] += err; srr -= err; err += drr;
      nxt[x]     += err; srr -= err;
      cur[x + 1] += srr;
    }

    buf[cos + x] = trc[touchar(cur[x])];
    buf[cos] = trc[touchar(cur[0])];

    cos += w;
    nos += w;
  }

  /* trunca toda a ultima linha */
  for (x = 0; x < w; x++)
    buf[cos + x] = trc[touchar(nxt[x])];

  free(cur);
  free(nxt);
}

/************************************************************\
* Faz dithering error difusion salvando o resultado em um    *
* mapa de cores.                                             *
\************************************************************/
static void difusion2map(int w, int h, uchar *buf, uchar *map, uchar *index)
{
  int x, y, k;
  int err, srr, drr;
  int *cur, *nxt;
  long int cos, nos;
  
  /* aloca espacco para duas linhas de trabalho */
  cur = (int *) malloc(w * sizeof(int));
  nxt = (int *) malloc(w * sizeof(int));

  nos = w;
  cos = 0;

  /* inicializa o buffer de proxima linha */
  for (x = 0; x < w; x++)
    nxt[x] = buf[x];

  for (y = 0; y < h-1; y++) {

    /* a proxima linha vira a linha corrente */
    swap(&cur, &nxt);

    /* pega a nova proxima linha */
    for (x = 0; x < w; x++)
      nxt[x] = buf[nos + x];

    for (x = 1; x < w-1; x++) {
      k = trc[touchar(cur[x])];
      map[cos + x] += index[k];
      srr = cur[x] - k;
      drr = srr >> 3;
      err = srr >> 4;
      nxt[x + 1] += err; srr -= err; err += drr;
      nxt[x - 1] += err; srr -= err; err += drr;
      nxt[x]     += err; srr -= err;
      cur[x + 1] += srr;
    }

    map[cos + x] += index[touchar(cur[x])];
    map[cos] += index[touchar(cur[0])];

    cos += w;
    nos += w;
  }

  /* trunca toda a ultima linha */
  for (x = 0; x < w; x++)
    map[cos + x] += index[touchar(nxt[x])];

  free(cur);
  free(nxt);
}

/************************************************************\
* Ajusta no mapa as cores do usuario que foram substituidas  *
* durante o dithering.                                       *
\************************************************************/
static void correct_map(int w, int h, uchar *pr, uchar *pg, uchar *pb, uchar *map)
{
  int c;
  long int os;
  int i, j;

  os = 0;
  for (i = 0; i < h; i++) {
    for (j = 0; j < w; j++) {
      for (c = 0; c < usr_n; c++) {
        if ((pr[os+j] == usr_r[c]) && (pg[os+j] == usr_g[c]) && (pb[os+j] == usr_b[c])) {
          map[os+j] = c + depth;
          break;
        }
      }
    }
    os += w;
  }
}

/************************************************************\
* Faz dithering error difusion salvando a imagem em um mapa  *
* de cores.                                                  *
\************************************************************/
void rgb_dither2map(int w, int h, uchar *pr, uchar *pg, uchar *pb, uchar *map)
{
  memset(map, 0, w*h);
  difusion2map(w, h, pr, map, rb);
  difusion2map(w, h, pg, map, gb);
  difusion2map(w, h, pb, map, bb);
  if (usr_n > 0) 
    correct_map(w, h, pr, pg, pb, map);
}

/************************************************************\
* Faz dithering error difusion salvando o resultado na pro-  *
* pria imagem.                                               *
\************************************************************/
void rgb_ditherinplace(int w, int h, uchar *pr, uchar *pg, uchar *pb)
{
  difusioninplace(w, h, pr);
  difusioninplace(w, h, pg);
  difusioninplace(w, h, pb);
}

/************************************************************\
* Retorna um ponteiro para a palheta de cores.               *
\************************************************************/
void rgb_syspal(long int **sys_pal, int *nc)
{
  *sys_pal = pal;
  if (depth == 216)
    *nc = 220;
  else
    *nc = 129;
}

/************************************************************\
* Define a palheta do usuario.                               *
\************************************************************/
void rgb_setusrpal(long int *usr_pal, int nc)
{
  int i;

  if (nc > 3) 
    nc = 3;

  for (i = 0; i < nc; i++) {
    pal[depth + i] = usr_pal[i];
    usr_r[i] = (gcuchar)((usr_pal[i] >> 16) & 0xff);
    usr_g[i] = (gcuchar)((usr_pal[i] >> 8) & 0xff);
    usr_b[i] = (gcuchar)(usr_pal[i] & 0xff);
  }

  usr_n = nc;
}

/************************************************************\
* Recupera a palheta do usuario.                             *
\************************************************************/
void rgb_getusrpal(long int **usr_pal, int *nc)
{
  *usr_pal = pal + depth;
  *nc = usr_n;
}

/************************************************************\
* Define o numero de cores no dithering.                     *
\************************************************************/
void rgb_depth(int new_depth)
{
  if (new_depth == 125) {
    depth = 125;
    pal = pal5;
    rb = rb5;
    gb = gb5;
    bb = bb5;
    trc = trc5;
  } else {
    depth = 216;
    pal = pal6;
    rb = rb6;
    gb = gb6;
    bb = bb6;
    trc = trc6;
  }
}
