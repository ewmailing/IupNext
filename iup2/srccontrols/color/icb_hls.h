/** \file
 * \brief iupcb control. HLS color system.
 *
 * See Copyright Notice in iup.h
 * $Id: icb_hls.h,v 1.1 2008-10-17 06:20:15 scuri Exp $
 */
 
#ifndef __ICB_HLS_H 
#define __ICB_HLS_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************\
* Implementa o modelo de cores HLS.                          *
\************************************************************/

/************************************************************\
* Converte do modelo HLS para o modelo RGB.                  *
\************************************************************/
void hls_hls2rgb(float h, float l, float s, float *pr, float *pg, float *pb);

/************************************************************\
* Converte do modelo RGB para o modelo HLS.                  *
\************************************************************/
void hls_rgb2hls(float r, float g, float b, float *ph, float *pl, float *ps);

/************************************************************\
* Gera um plasma circular de todos os valores de h.          *
\************************************************************/
void hls_hplasma(int w, int h, int cl, uchar *ur, uchar *ug, uchar *ub);

/************************************************************\
* Gera um plasma triangular de todos os valores de l e s     *
* para um valor fixo de h.                                   *
\************************************************************/
void hls_lsplasma(int iw, int ih, float h, uchar *ur, uchar *ug, uchar *ub);

/************************************************************\
* Gera um plasma de todos os valores de s para um h e um l   *
* fixos.                                                     *
\************************************************************/
void hls_splasma(int iw, int ih, float h, float l,  uchar *ur,
uchar *ug, uchar *ub);

/************************************************************\
* Gera um plasma de todos os valores de l para um h e um s   *
* fixos.                                                     *
\************************************************************/
void hls_lplasma(int iw, int ih, float h, float s,  uchar *ur,
uchar *ug, uchar *ub);

/************************************************************\
* Configura as cores de sombra e fundo do usuario.           *
\************************************************************/
void hls_shade(int ur, int ug, int ub, int url, int ugl, int ubl, 
int urd, int ugd, int ubd);

#ifdef __cplusplus
}
#endif

#endif
