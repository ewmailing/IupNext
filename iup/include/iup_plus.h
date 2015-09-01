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

  public:
    Handle(Ihandle* ref_ih)
    {
      ih = ref_ih;
    }

    Ihandle* GetHandle() const { return ih; }
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
  class GLControls
  {
  public:
    static void Open() { IupGLControlsOpen(); }
  };
  class GLSubCanvas : public Handle
  {
  public:
    GLSubCanvas() : Handle(IupGLSubCanvas()) {}
  };
  class GLSeparator : public Handle
  {
  public:
    GLSeparator() : Handle(IupGLSeparator()) {}
  };
  class GLProgressBar : public Handle
  {
  public:
    GLProgressBar() : Handle(IupGLProgressBar()) {}
  };
  class GLVal : public Handle
  {
  public:
    GLVal() : Handle(IupGLVal()) {}
  };
  class GLLabel : public Handle
  {
  public:
    GLLabel(const char* title = 0) : Handle(IupGLLabel(title)) {}
  };
  class GLButton : public Handle
  {
  public:
    GLButton(const char* title = 0) : Handle(IupGLButton(title)) {}
  };
  class GLToggle : public Handle
  {
  public:
    GLToggle(const char* title = 0) : Handle(IupGLToggle(title)) {}
  };
  class GLLink : public Handle
  {
  public:
    GLLink(const char *url = 0, const char* title = 0) : Handle(IupGLLink(url, title)) {}
  };
  class GLFrame : public Handle
  {
  public:
    GLFrame(Handle& child) : Handle(IupGLFrame(child.GetHandle())) {}
    GLFrame() : Handle(IupGLFrame(0)) {}
  };
  class GLExpander : public Handle
  {
  public:
    GLExpander(Handle& child) : Handle(IupGLExpander(child.GetHandle())) {}
    GLExpander() : Handle(IupGLExpander(0)) {}
  };
  class GLScrollBox : public Handle
  {
  public:
    GLScrollBox(Handle& child) : Handle(IupGLScrollBox(child.GetHandle())) {}
    GLScrollBox() : Handle(IupGLScrollBox(0)) {}
  };
  class GLSizeBox : public Handle
  {
  public:
    GLSizeBox(Handle& child) : Handle(IupGLSizeBox(child.GetHandle())) {}
    GLSizeBox() : Handle(IupGLSizeBox(0)) {}
  };
  class GLCanvasBox : public Handle
  {
  public:
    GLCanvasBox() : Handle(IupGLCanvasBox(0)) {}
    GLCanvasBox(Handle& child) : Handle(IupGLCanvasBox(child.GetHandle(), 0)) {}
    GLCanvasBox(Handle* child, int count) : Handle(IupGLCanvasBox(0)) 
    {
      for (int i = 0; i < count; i++)
        IupAppend(ih, child[i].GetHandle());
    }
  };
  class Plot : public Handle
  {
  public:
    Plot() : Handle(IupPlot()) {}

    static void Open() { IupPlotOpen(); }

    void Begin(int strXdata)
    {
      IupPlotBegin(ih, strXdata);
    }
    void Add(double x, double y)
    {
      IupPlotAdd(ih, x, y);
    }
    void AddStr(const char* x, double y)
    {
      IupPlotAddStr(ih, x, y);
    }
    void AddSegment(double x, double y)
    {
      IupPlotAddSegment(ih, x, y);
    }
    int End(Ihandle *ih)
    {
      return IupPlotEnd(ih);
    }

    int LoadData(const char* filename, int strXdata)
    {
      return IupPlotLoadData(ih, filename, strXdata);
    }

    int SetFormula(int sample_count, const char* formula, const char* init)
    {
      return IupPlotSetFormula(ih, sample_count, formula, init);
    }

    void Insert(int ds_index, int sample_index, double x, double y)
    {
      IupPlotInsert(ih, ds_index, sample_index, x, y);
    }
    void InsertStr(int ds_index, int sample_index, const char* x, double y)
    {
      IupPlotInsertStr(ih, ds_index, sample_index, x, y);
    }
    void InsertSegment(int ds_index, int sample_index, double x, double y)
    {
      IupPlotInsertSegment(ih, ds_index, sample_index, x, y);
    }

    void InsertStrSamples(int ds_index, int sample_index, const char** x, double* y, int count)
    {
      IupPlotInsertStrSamples(ih, ds_index, sample_index, x, y, count);
    }
    void InsertSamples(int ds_index, int sample_index, double *x, double *y, int count)
    {
      IupPlotInsertSamples(ih, ds_index, sample_index, x, y, count);
    }

    void AddSamples(int ds_index, double *x, double *y, int count)
    {
      IupPlotAddSamples(ih, ds_index, x, y, count);
    }
    void AddStrSamples(int ds_index, const char** x, double* y, int count)
    {
      IupPlotAddStrSamples(ih, ds_index, x, y, count);
    }

    void GetSample(int ds_index, int sample_index, double &x, double &y)
    {
      IupPlotGetSample(ih, ds_index, sample_index, &x, &y);
    }
    void GetSampleStr(int ds_index, int sample_index, const char* &x, double &y)
    {
      IupPlotGetSampleStr(ih, ds_index, sample_index, &x, &y);
    }
    int GetSampleSelection(int ds_index, int sample_index)
    {
      return IupPlotGetSampleSelection(ih, ds_index, sample_index);
    }
    void SetSample(int ds_index, int sample_index, double x, double y)
    {
      IupPlotSetSample(ih, ds_index, sample_index, x, y);
    }
    void SetSampleStr(int ds_index, int sample_index, const char* x, double y)
    {
      IupPlotSetSampleStr(ih, ds_index, sample_index, x, y);
    }
    void SetSampleSelection(int ds_index, int sample_index, int selected)
    {
      IupPlotSetSampleSelection(ih, ds_index, sample_index, selected);
    }

    void Transform(double x, double y, double &cnv_x, double &cnv_y)
    {
      IupPlotTransform(ih, x, y, &cnv_x, &cnv_y);
    }
    void TransformTo(double cnv_x, double cnv_y, double &x, double &y)
    {
      IupPlotTransformTo(ih, cnv_x, cnv_y, &x, &y);
    }

    int FindSample(double cnv_x, double cnv_y, int &ds_index, int &sample_index)
    {
      return IupPlotFindSample(ih, cnv_x, cnv_y, &ds_index, &sample_index);
    }

    void PaintTo(cd::Canvas& cd_canvas)
    {
      IupPlotPaintTo(ih, cd_canvas.GetHandle());
    }
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

  class Config: public Handle
  {
  public:
    Config(): Handle(IupConfig()) { }

    int LoadConfig()
    {
      return IupConfigLoad(ih);
    }
    int SaveConfig()
    {
      return IupConfigSave(ih);
    }

    void SetVariableStrId(const char* group, const char* key, int id, const char* value)
    {
      IupConfigSetVariableStrId(ih, group, key, id, value);
    }
    void SetVariableIntId(const char* group, const char* key, int id, int value)
    {
      IupConfigSetVariableIntId(ih, group, key, id, value);
    }
    void SetVariableDoubleId(const char* group, const char* key, int id, double value)
    {
      IupConfigSetVariableDoubleId(ih, group, key, id, value);
    }
    void SetVariableStr(const char* group, const char* key, const char* value)
    {
      IupConfigSetVariableStr(ih, group, key, value);
    }
    void SetVariableInt(const char* group, const char* key, int value)
    {
      IupConfigSetVariableInt(ih, group, key, value);
    }
    void SetVariableDouble(const char* group, const char* key, double value)
    {
      IupConfigSetVariableDouble(ih, group, key, value);
    }

    char* GetVariableStr(const char* group, const char* key)
    {
      return (char*)IupConfigGetVariableStr(ih, group, key);
    }
    int GetVariableInt(const char* group, const char* key)
    {
      return IupConfigGetVariableInt(ih, group, key);
    }
    double GetVariableDouble(const char* group, const char* key)
    {
      return IupConfigGetVariableDouble(ih, group, key);
    }
    char* GetVariableStrId(const char* group, const char* key, int id)
    {
      return (char*)IupConfigGetVariableStrId(ih, group, key, id);
    }
    int GetVariableIntId(const char* group, const char* key, int id)
    {
      return IupConfigGetVariableIntId(ih, group, key, id);
    }
    double GetVariableDoubleId(const char* group, const char* key, int id)
    {
      return IupConfigGetVariableDoubleId(ih, group, key, id);
    }

    char* GetVariableStrDef(const char* group, const char* key, const char* def)
    {
      return (char*)IupConfigGetVariableStrDef(ih, group, key, def);
    }
    int GetVariableIntDef(const char* group, const char* key, int def)
    {
      return IupConfigGetVariableIntDef(ih, group, key, def);
    }
    double GetVariableDoubleDef(const char* group, const char* key, double def)
    {
      return IupConfigGetVariableDoubleDef(ih, group, key, def);
    }
    char* GetVariableStrIdDef(const char* group, const char* key, int id, const char* def)
    {
      return (char*)IupConfigGetVariableStrIdDef(ih, group, key, id, def);
    }
    int GetVariableIntIdDef(const char* group, const char* key, int id, int def)
    {
      return IupConfigGetVariableIntIdDef(ih, group, key, id, def);
    }
    double GetVariableDoubleIdDef(const char* group, const char* key, int id, double def)
    {
      return IupConfigGetVariableDoubleIdDef(ih, group, key, id, def);
    }

    void RecentInit(Ihandle* menu, Icallback recent_cb, int max_recent)
    {
      IupConfigRecentInit(ih, menu, recent_cb, max_recent);
    }
    void RecentUpdate(const char* filename)
    {
      IupConfigRecentUpdate(ih, filename);
    }

    void DialogShow(Ihandle* dialog, const char* name)
    {
      IupConfigDialogShow(ih, dialog, name);
    }
    void DialogClosed(Ihandle* dialog, const char* name)
    {
      IupConfigDialogClosed(ih, dialog, name);
    }
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
      canvas = cdCreateCanvas(CD_IUP, iup_canvas.GetHandle());
    }
  };
  class CanvasIupDoubleBuffer : public Canvas
  {
  public:
    CanvasIupDoubleBuffer(iup::Canvas& iup_canvas)
      : Canvas()
    {
      canvas = cdCreateCanvas(CD_IUPDBUFFER, iup_canvas.GetHandle());
    }
  };
  class CanvasIupDoubleBufferRGB : public Canvas
  {
  public:
    CanvasIupDoubleBufferRGB(iup::Canvas& iup_canvas)
      : Canvas()
    {
      canvas = cdCreateCanvas(CD_IUPDBUFFERRGB, iup_canvas.GetHandle());
    }
  };
}

#endif
