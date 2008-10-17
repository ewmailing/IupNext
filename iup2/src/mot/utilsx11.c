/* Name:        src/unix/utilsx11.cpp
 * Purpose:     Miscellaneous X11 functions
 * Author:      Mattia Barbon, Vaclav Slavik, Robert Roebling
 * Modified by:
 * Created:     25.03.02
 * RCS-ID:      $Id: utilsx11.c,v 1.1 2008-10-17 06:19:26 scuri Exp $
 * Copyright:   (c) wxWindows team
 * Licence:     wxWindows licence
*/

#ifdef __VMS
#pragma message disable nosimpint
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#ifdef __VMS
#pragma message enable nosimpint
#endif

#include <limits.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Various X11 Atoms used in this file: */
static Atom _NET_WM_ICON = 0;
static Atom _NET_WM_STATE = 0;
static Atom _NET_WM_STATE_FULLSCREEN = 0;
static Atom _NET_WM_STATE_STAYS_ON_TOP = 0;
static Atom _NET_WM_WINDOW_TYPE = 0;
static Atom _NET_WM_WINDOW_TYPE_NORMAL = 0;
static Atom _KDE_NET_WM_WINDOW_TYPE_OVERRIDE = 0;
static Atom _WIN_LAYER = 0;
static Atom KWIN_RUNNING = 0;
static Atom _NET_SUPPORTING_WM_CHECK = 0;
static Atom _NET_SUPPORTED = 0;

#define MAKE_ATOM(name, display) \
    if (name == 0) name = XInternAtom((display), #name, False)

enum 
{
    wxX11_FS_AUTODETECT = 0,
    wxX11_FS_WMSPEC,
    wxX11_FS_KDE,
    wxX11_FS_GENERIC
};

/* Is the window mapped? */
static int IsMapped(Display *display, Window window)
{
    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);
    return (attr.map_state != IsUnmapped);
}

/* ----------------------------------------------------------------------------
 Fullscreen mode:
 ----------------------------------------------------------------------------

 NB: Setting fullscreen mode under X11 is a complicated matter. There was
     no standard way of doing it until recently. ICCCM doesn't know the
     concept of fullscreen windows and the only way to make a window
     fullscreen is to remove decorations, resize it to cover entire screen
     and set WIN_LAYER_ABOVE_DOCK.

     This doesn't always work, though. Specifically, at least kwin from 
     KDE 3 ignores the hint. The only way to make kwin accept our request
     is to emulate the way Qt does it. That is, unmap the window, set
     _NET_WM_WINDOW_TYPE to _KDE_NET_WM_WINDOW_TYPE_OVERRIDE (KDE extension),
     add _NET_WM_STATE_STAYS_ON_TOP (ditto) to _NET_WM_STATE and map
     the window again.

     Version 1.2 of Window Manager Specification (aka wm-spec aka
     Extended Window Manager Hints) introduced _NET_WM_STATE_FULLSCREEN
     window state which provides cleanest and simplest possible way of
     making a window fullscreen. WM-spec is a de-facto standard adopted
     by GNOME and KDE folks, but _NET_WM_STATE_FULLSCREEN isn't yet widely
     supported. As of January 2003, only GNOME 2's default WM Metacity 
     implements, KDE will support it from version 3.2. At toolkits level,
     GTK+ >= 2.1.2 uses it as the only method of making windows fullscreen
     (that's why wxGTK will *not* switch to using gtk_window_fullscreen
     unless it has better compatiblity with older WMs).

     
     This is what wxWindows does in wxSetFullScreenStateX11:
       1) if _NET_WM_STATE_FULLSCREEN is supported, use it
       2) otherwise try WM-specific hacks (KDE, IceWM)
       3) use _WIN_LAYER and hope that the WM will recognize it
     The code was tested with:
     twm, IceWM, WindowMaker, Metacity, kwin, sawfish, lesstif-mwm
*/

#define  WIN_LAYER_NORMAL       4
#define  WIN_LAYER_ABOVE_DOCK  10

static void wxWinHintsSetLayer(Display *display, Window rootWnd,
                               Window window, int layer)
{
    XEvent xev;

    MAKE_ATOM( _WIN_LAYER, display );

    if (IsMapped(display, window))
    {
        xev.type = ClientMessage;
        xev.xclient.type = ClientMessage;
        xev.xclient.window = window;
        xev.xclient.message_type = _WIN_LAYER;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = (long)layer;
        xev.xclient.data.l[1] = CurrentTime;

        XSendEvent(display, rootWnd, False,
                   SubstructureNotifyMask, (XEvent*) &xev);
    }
    else
    {
        long data[1];

        data[0] = layer;
        XChangeProperty(display, window,
                        _WIN_LAYER, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *)data, 1);
    }
}

static int wxQueryWMspecSupport(Display *display, Window rootWnd, Atom feature)
{
    /* FIXME: We may want to cache these checks. Note that we can't simply
            remember the results in global variable because the WM may go
            away and be replaced by another one! One possible approach
            would be invalidate the case every 15 seconds or so. Since this
            code is currently only used by wxTopLevelWindow::ShowFullScreen,
            it is not important that it is not optimized.
    
            If the WM supports ICCCM (i.e. the root window has
            _NET_SUPPORTING_WM_CHECK property that points to a WM-owned
            window), we could watch for DestroyNotify event on the window
            and invalidate our cache when the windows goes away (= WM
            is replaced by another one). This is what GTK+ 2 does.
            Let's do it only if it is needed, it requires changes to
            the event loop.
    */
  
    Atom type;
    Window *wins;
    Atom *atoms;
    int format, i;
    unsigned long after;
    unsigned long nwins, natoms;
    
    MAKE_ATOM(_NET_SUPPORTING_WM_CHECK, display);
    MAKE_ATOM(_NET_SUPPORTED, display);

    /* Is the WM ICCCM supporting? */
    XGetWindowProperty(display, rootWnd,
                       _NET_SUPPORTING_WM_CHECK, 0, LONG_MAX,
                       False, XA_WINDOW, &type, &format, &nwins,
                       &after, (unsigned char **)&wins);
    if ( type != XA_WINDOW || nwins <= 0 || wins[0] == None )
       return FALSE;
    XFree(wins);

    /* Query for supported features: */
    XGetWindowProperty(display, rootWnd,
                       _NET_SUPPORTED, 0, LONG_MAX,
                       False, XA_ATOM, &type, &format, &natoms,
                       &after, (unsigned char **)&atoms);
    if ( type != XA_ATOM || atoms == NULL )
        return FALSE;

    /* Lookup the feature we want: */
    for (i = 0; i < natoms; i++)
    {
        if ( atoms[i] == feature )
        {
            XFree(atoms);
            return TRUE;
        }
    }
    XFree(atoms);
    return FALSE;
}

#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1

static void wxWMspecSetState(Display *display, Window rootWnd,
                             Window window, int operation, Atom state)
{
    MAKE_ATOM(_NET_WM_STATE, display);
  
    if ( IsMapped(display, window) )
    {
        XEvent xev;
        xev.type = ClientMessage;
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = display;
        xev.xclient.window = window;
        xev.xclient.message_type = _NET_WM_STATE;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = operation;
        xev.xclient.data.l[1] = state;
        xev.xclient.data.l[2] = None;
  
        XSendEvent(display, rootWnd,
                   False,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   &xev);
    }
    /* FIXME - must modify _NET_WM_STATE property list if the window */
    /*         wasn't mapped!                                        */
}

static void wxWMspecSetFullscreen(Display *display, Window rootWnd,
                                  Window window, int fullscreen)
{
    MAKE_ATOM(_NET_WM_STATE_FULLSCREEN, display);
    wxWMspecSetState(display, rootWnd,
                     window,
                     fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE,
                      _NET_WM_STATE_FULLSCREEN);
}


/* Is the user running KDE's kwin window manager? At least kwin from KDE 3 */
/* sets KWIN_RUNNING property on the root window.                          */
static int wxKwinRunning(Display *display, Window rootWnd)
{
    long *data;
    Atom type;
    int format, retval;
    unsigned long nitems, after;

    MAKE_ATOM(KWIN_RUNNING, display);

    if (XGetWindowProperty(display, rootWnd,
                           KWIN_RUNNING, 0, 1, False, KWIN_RUNNING,
                           &type, &format, &nitems, &after,
                           (unsigned char**)&data) != Success)
    {
        return FALSE;
    }

    retval = (type == KWIN_RUNNING &&
              nitems == 1 && data && data[0] == 1);
    XFree(data);
    return retval;
}

/* KDE's kwin is Qt-centric so much than no normal method of fullscreen */
/* mode will work with it. We have to carefully emulate the Qt way.     */
static void wxSetKDEFullscreen(Display *display, Window rootWnd,
                               Window w, int fullscreen)
{
    long data[2];
    unsigned lng;
    int wasMapped;

    MAKE_ATOM(_NET_WM_WINDOW_TYPE, display);
    MAKE_ATOM(_NET_WM_WINDOW_TYPE_NORMAL, display);
    MAKE_ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE, display);
    MAKE_ATOM(_NET_WM_STATE_STAYS_ON_TOP, display);

    if (fullscreen)
    {
        data[0] = _KDE_NET_WM_WINDOW_TYPE_OVERRIDE;
        data[1] = _NET_WM_WINDOW_TYPE_NORMAL;
        lng = 2;
    }
    else
    {
        data[0] = _NET_WM_WINDOW_TYPE_NORMAL;
        data[1] = None;
        lng = 1;
    }

    /* it is neccessary to unmap the window, otherwise kwin will ignore us: */
    XSync(display, False);
    
    wasMapped = IsMapped(display, w);
    if (wasMapped)
    {
        XUnmapWindow(display, w);
        XSync(display, False);
    }

    XChangeProperty(display, w, _NET_WM_WINDOW_TYPE, XA_ATOM, 32,
	                PropModeReplace, (unsigned char *) &data, lng);
    XSync(display, False);

    if (wasMapped)
    {
        XMapRaised(display, w);
        XSync(display, False);
    }
    
    wxWMspecSetState(display, rootWnd, w, 
                     fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE,
                     _NET_WM_STATE_STAYS_ON_TOP);
    XSync(display, False);
}

static int wxGetFullScreenMethodX11(Display* disp, Window root)
{
    /* if WM supports _NET_WM_STATE_FULLSCREEN from wm-spec 1.2, use it: */
    MAKE_ATOM(_NET_WM_STATE_FULLSCREEN, disp);
    if (wxQueryWMspecSupport(disp, root, _NET_WM_STATE_FULLSCREEN))
        return wxX11_FS_WMSPEC;
    
    /* if the user is running KDE's kwin WM, use a legacy hack because */
    /* kwin doesn't understand any other method:                       */
    if (wxKwinRunning(disp, root))
        return wxX11_FS_KDE;
    
    return wxX11_FS_GENERIC;
}

void wxSetFullScreenStateX11(Display* disp, Window root, Window wnd, int full)
{
    /* NB: please see the comment under "Fullscreen mode:" title above */
    /*     for implications of changing this code.                     */

    int method = wxGetFullScreenMethodX11(disp, root);

    switch (method)
    {
        case wxX11_FS_WMSPEC:
            wxWMspecSetFullscreen(disp, root, wnd, full);
            break;
        case wxX11_FS_KDE:
            wxSetKDEFullscreen(disp, root, wnd, full);
            break;
        default:
            wxWinHintsSetLayer(disp, root, wnd, full? WIN_LAYER_ABOVE_DOCK : WIN_LAYER_NORMAL);
            break;
    }
}

