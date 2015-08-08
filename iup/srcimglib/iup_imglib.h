#ifndef __IUP_IMGLIB_H 
#define __IUP_IMGLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
void iupImglibBaseLibWin16x16Open(void);     /* Used only by the Win32 driver */
#endif
#ifndef WIN32
void iupImglibBaseLibMot16x16Open(void);    /* Used only by the Motif driver */
#endif

void iupImglibBaseLibGtk16x16Open(void);  /* Used only by the GTK driver in UNIX or Windows */

#ifndef WIN32
void iupImglibLogosMot32x32Open(void);      /* Used only by the Motif driver */
void iupImglibLogosMot48x48Open(void);      /* Used only by the Motif driver */
#endif

void iupImglibLogos32x32Open(void);

#ifdef IUP_IMGLIB_LARGE
void iupImglibLogos48x48Open(void);
void iupImglibIconsGtk48x48Open(void);

#ifdef WIN32
void iupImglibIconsWin48x48Open(void);
#endif
#endif  

#ifdef __cplusplus
}
#endif

#endif
