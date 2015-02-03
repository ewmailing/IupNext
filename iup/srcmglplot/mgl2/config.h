#ifndef _MGL_CONFIG_H_
#define _MGL_CONFIG_H_

#define MGL_USE_DOUBLE	1

#ifdef WIN32	// a man ask to use built-in font under Windows
#define MGL_DEF_FONT_NAME	""
#else
#define MGL_DEF_FONT_NAME	""
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define MGL_SYS_NAN		0
#define MGL_HAVE_TYPEOF	0
#define MGL_HAVE_PTHREAD	0
#define MGL_HAVE_ATTRIBUTE	0
#define MGL_HAVE_C99_COMPLEX	0
#else
#define MGL_HAVE_TYPEOF	${MGL_HAVE_TYPEOF}
#define MGL_SYS_NAN		${MGL_HAVE_NAN_INF}
#define MGL_HAVE_PTHREAD	${MGL_HAVE_PTHREAD}
#define MGL_HAVE_ATTRIBUTE	${MGL_HAVE_ATTRIBUTE}
#define MGL_HAVE_C99_COMPLEX	${MGL_HAVE_C99_COMPLEX}
#endif

#define MGL_HAVE_LTDL	${MGL_HAVE_LTDL}
#define MGL_HAVE_RVAL	${MGL_HAVE_RVAL}
#define MGL_HAVE_ZLIB	${MGL_HAVE_ZLIB}
#define MGL_HAVE_PNG	${MGL_HAVE_PNG}
#define MGL_HAVE_GSL	${MGL_HAVE_GSL}
#define MGL_HAVE_OPENGL	1
#define MGL_HAVE_OMP	${MGL_HAVE_OMP}
#define MGL_HAVE_JPEG	${MGL_HAVE_JPEG}
#define MGL_HAVE_GIF	${MGL_HAVE_GIF}
#define MGL_HAVE_PDF	${MGL_HAVE_PDF}
#define MGL_HAVE_HDF4	${MGL_HAVE_HDF4}
#define MGL_HAVE_HDF5	${MGL_HAVE_HDF5}
#define MGL_FONT_PATH	""

#endif
