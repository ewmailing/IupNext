/** \file
 * \brief iupcb control. Error difusion dither..
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __ICB_RGB_H 
#define __ICB_RGB_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************\
* Implementa dithering error difusion com uma palhera fixa   *
* de 216 cores.                                              *
\************************************************************/

/************************************************************\
* Definicoes dos tipos de dados usados.                      *
\************************************************************/
typedef unsigned char gcuchar;
#define uchar gcuchar
#define touchar(x) ((uchar) (x < 0 ? 0 : (x > 255 ? 255 : x)))

/************************************************************\
* Prototipos das funccoes exportadas.                        *
\************************************************************/

/************************************************************\
* Retorna um ponteiro para a palheta de cores.               *
\************************************************************/
void rgb_syspal(long int **sys_pal, int *nc);

/************************************************************\
* Define a palheta de cores do usuario.                      *
\************************************************************/
void rgb_setusrpal(long int *usr_pal, int nc);

/************************************************************\
* Recupera a palheta do usuario.                             *
\************************************************************/
void rgb_getusrpal(long int **usr_pal, int *nc);

/************************************************************\
* Faz dithering error difusion salvando o resultado na pro-  *
* pria imagem.                                               *
\************************************************************/
void rgb_ditherinplace(int w, int h, uchar *pr, uchar *pg, uchar *pb);

/************************************************************\
* Faz dithering error difusion salvando a imagem em um mapa  *
* de cores.                                                  *
\************************************************************/
void rgb_dither2map(int w, int h, uchar *pr, uchar *pg, uchar *pb, uchar *map);

/************************************************************\
* Define o numero de cores no dithering.                     *
\************************************************************/
void rgb_depth(int depth);

#ifdef __cplusplus
}
#endif

#endif
