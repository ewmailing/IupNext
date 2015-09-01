/** \file
 * \brief Name space for C++ high level API
 *
 * See Copyright Notice in iup.h
 */
 
#ifndef __IUP_PLUS_H
#define __IUP_PLUS_H


#include <cd_plus.h>

#include "iup.h"
#include "iupkey.h"
#include "iup_class_cbs.hpp"
#include "iupcontrols.h"
#include "iupmatrixex.h"
#include "iupgl.h"
  #include "iupglcontrols.h"
  #include "iupim.h"
  #include "iup_config.h"
  #include "iup_mglplot.h"
  #include "iup_plot.h"
#include "iupole.h"
#include "iupweb.h"
#include "iup_scintilla.h"
#include "iuptuio.h"


namespace cd
{
  class CanvasIup;
  class CanvasIupDoubleBuffer;
  class CanvasIupDoubleBufferRGB;
}


/** \brief Name space for C++ high level API
 *
 * \par
 * Defines wrapper classes for all C structures.
 *
 * See \ref iup_plus.h
 */
namespace iup
{
  inline char* Version()
  {
    return IupVersion();
  }
  inline char* VersionDate()
  {
    return IupVersionDate();
  }
  inline int VersionNumber()
  {
    return IupVersionNumber();
  }


  class Handle
  {
  protected:
    Ihandle* ih;

    friend class cd::CanvasIup;
    friend class cd::CanvasIupDoubleBuffer;
    friend class cd::CanvasIupDoubleBufferRGB;

  public:
    Handle(Ihandle* ref_ih)
    {
      ih = ref_ih;
    }
  };
  

  class Canvas : public Handle
  {
  public:
    Canvas() : Handle(IupCanvas(0)) {}
  };

  class GLCanvas : public Handle
  {
  public:
    GLCanvas() : Handle(IupGLCanvas(0)) {}

    static void Open() { IupGLCanvasOpen(); }

    void MakeCurrent()
    {
      IupGLMakeCurrent(ih);
    }
    int IsCurrent()
    {
      return IupGLIsCurrent(ih);
    }
    void SwapBuffers()
    {
      IupGLSwapBuffers(ih);
    }
    void Palette(int index, float r, float g, float b)
    {
      IupGLPalette(ih, index, r, g, b);
    }
    void UseFont(int first, int count, int list_base)
    {
      IupGLUseFont(ih, first, count, list_base);
    }

    static void Wait(int gl) { IupGLWait(gl); }
  };

  class Controls
  {
  public:
    static void Open() { IupControlsOpen(); }
  };
  class Dial : public Handle
  {
  public:
    Dial(const char* orientation = "HORIZONTAL") : Handle(IupDial(orientation)) {}
  };
  class Gauge : public Handle
  {
  public:
    Gauge() : Handle(IupGauge()) {}
  };
  class ColorBrowser : public Handle
  {
  public:
    ColorBrowser() : Handle(IupColorBrowser()) {}
  };
  class Cells : public Handle
  {
  public:
    Cells() : Handle(IupCells()) {}
  };
  class Colorbar : public Handle
  {
  public:
    Colorbar() : Handle(IupColorbar()) {}
  };
  class Matrix : public Handle
  {
  public:
    Matrix() : Handle(IupMatrix(0)) {}

    void SetFormula(int col, const char* formula, const char* init = 0)
    {
      IupMatrixSetFormula(ih, col, formula, init);
    }
    void SetDynamic(Ihandle* ih, const char* init = 0)
    {
      IupMatrixSetDynamic(ih, init);
    }
  };
  class MatrixList : public Handle
  {
  public:
    MatrixList() : Handle(IupMatrixList()) {}
  };
  class MatrixEx : public Handle
  {
  public:
    MatrixEx() : Handle(IupMatrixEx()) {}

    static void Open() { IupMatrixExOpen(); }
  };
  class OleControl : public Handle
  {
  public:
    OleControl(const char* progid) : Handle(IupOleControl(progid)) {}

    static void Open() { IupOleControlOpen(); }
  };
  class WebBrowser : public Handle
  {
  public:
    WebBrowser() : Handle(IupWebBrowser()) {}

    static void Open() { IupWebBrowserOpen(); }
  };
  class Scintilla : public Handle
  {
  public:
    Scintilla(): Handle(IupScintilla()) {}

    static void Open() { IupScintillaOpen(); }
  };
  class TuioClient : public Handle
  {
  public:
    TuioClient(int port) : Handle(IupTuioClient(port)) {}

    static void Open() { IupTuioOpen(); }
  };
}

namespace cd
{
  class CanvasIup : public Canvas
  {
  public:
    CanvasIup(iup::Canvas& iup_canvas)
      : Canvas()
    {
      canvas = cdCreateCanvas(CD_IUP, iup_canvas.ih);
    }
  };
  class CanvasIupDoubleBuffer : public Canvas
  {
  public:
    CanvasIupDoubleBuffer(iup::Canvas& iup_canvas)
      : Canvas()
    {
      canvas = cdCreateCanvas(CD_IUPDBUFFER, iup_canvas.ih);
    }
  };
  class CanvasIupDoubleBufferRGB : public Canvas
  {
  public:
    CanvasIupDoubleBufferRGB(iup::Canvas& iup_canvas)
      : Canvas()
    {
      canvas = cdCreateCanvas(CD_IUPDBUFFERRGB, iup_canvas.ih);
    }
  };
}

#endif
