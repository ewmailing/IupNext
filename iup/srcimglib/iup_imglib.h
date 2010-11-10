#ifndef __IUP_IMGLIB_H 
#define __IUP_IMGLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
void iupImglibBitmapsOpen(void);     /* Used only by the Win32 driver in Windows */
#endif
#ifndef WIN32
void iupImglibBitmaps8Open(void);    /* Used only by the Motif driver in UNIX */
#endif
void iupImglibBitmapsGtkOpen(void);  /* Used only by the GTK driver in UNIX or Windows */
#ifndef WIN32
void iupImglibIcons8Open(void);      /* Used only by the Motif driver in UNIX */
void iupImglibLogos8Open(void);      /* Used only by the Motif driver in UNIX */
#endif
void iupImglibIconsOpen(void);
void iupImglibLogosOpen(void);
#ifdef WIN32
void iupImglibLogosWin32Open(void);
void iupImglibLogosWin32MsgOpen(void);
#endif
void iupImglibLogosGtkOpen(void);


#ifdef __cplusplus
}
#endif

#endif
