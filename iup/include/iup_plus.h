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


#if 0
int       IupOpen(int *argc, char ***argv);
void      IupClose(void);
void      IupImageLibOpen(void);

int       IupMainLoop(void);
int       IupLoopStep(void);
int       IupLoopStepWait(void);
int       IupMainLoopLevel(void);
void      IupFlush(void);
void      IupExitLoop(void);

int       IupRecordInput(const char* filename, int mode);
int       IupPlayInput(const char* filename);

void      IupUpdate(Ihandle* ih);
void      IupUpdateChildren(Ihandle* ih);
void      IupRedraw(Ihandle* ih, int children);
void      IupRefresh(Ihandle* ih);
void      IupRefreshChildren(Ihandle* ih);

int       IupHelp(const char* url);
char*     IupLoad(const char *filename);
char*     IupLoadBuffer(const char *buffer);

char*     IupVersion(void);
char*     IupVersionDate(void);
int       IupVersionNumber(void);

void      IupSetLanguage(const char *lng);
char*     IupGetLanguage(void);
void      IupSetLanguageString(const char* name, const char* str);
void      IupStoreLanguageString(const char* name, const char* str);
char*     IupGetLanguageString(const char* name);
void      IupSetLanguagePack(Ihandle* ih);

void      IupDestroy(Ihandle* ih);
void      IupDetach(Ihandle* child);
Ihandle*  IupAppend(Ihandle* ih, Ihandle* child);
Ihandle*  IupInsert(Ihandle* ih, Ihandle* ref_child, Ihandle* child);
Ihandle*  IupGetChild(Ihandle* ih, int pos);
int       IupGetChildPos(Ihandle* ih, Ihandle* child);
int       IupGetChildCount(Ihandle* ih);
Ihandle*  IupGetNextChild(Ihandle* ih, Ihandle* child);
Ihandle*  IupGetBrother(Ihandle* ih);
Ihandle*  IupGetParent(Ihandle* ih);
Ihandle*  IupGetDialog(Ihandle* ih);
Ihandle*  IupGetDialogChild(Ihandle* ih, const char* name);
int       IupReparent(Ihandle* ih, Ihandle* new_parent, Ihandle* ref_child);

int       IupPopup(Ihandle* ih, int x, int y);
int       IupShow(Ihandle* ih);
int       IupShowXY(Ihandle* ih, int x, int y);
int       IupHide(Ihandle* ih);
int       IupMap(Ihandle* ih);
void      IupUnmap(Ihandle *ih);

void      IupResetAttribute(Ihandle *ih, const char* name);
int       IupGetAllAttributes(Ihandle* ih, char** names, int n);
Ihandle*  IupSetAtt(const char* handle_name, Ihandle* ih, const char* name, ...);
Ihandle*  IupSetAttributes(Ihandle* ih, const char *str);
char*     IupGetAttributes(Ihandle* ih);

void      IupSetAttribute(Ihandle* ih, const char* name, const char* value);
void      IupSetStrAttribute(Ihandle* ih, const char* name, const char* value);
void      IupSetStrf(Ihandle* ih, const char* name, const char* format, ...);
void      IupSetInt(Ihandle* ih, const char* name, int value);
void      IupSetFloat(Ihandle* ih, const char* name, float value);
void      IupSetDouble(Ihandle* ih, const char* name, double value);
void      IupSetRGB(Ihandle *ih, const char* name, unsigned char r, unsigned char g, unsigned char b);

char*     IupGetAttribute(Ihandle* ih, const char* name);
int       IupGetInt(Ihandle* ih, const char* name);
int       IupGetInt2(Ihandle* ih, const char* name);
int       IupGetIntInt(Ihandle *ih, const char* name, int *i1, int *i2);
float     IupGetFloat(Ihandle* ih, const char* name);
double    IupGetDouble(Ihandle* ih, const char* name);
void      IupGetRGB(Ihandle *ih, const char* name, unsigned char *r, unsigned char *g, unsigned char *b);

void  IupSetAttributeId(Ihandle *ih, const char* name, int id, const char *value);
void  IupSetStrAttributeId(Ihandle *ih, const char* name, int id, const char *value);
void  IupSetStrfId(Ihandle *ih, const char* name, int id, const char* format, ...);
void  IupSetIntId(Ihandle* ih, const char* name, int id, int value);
void  IupSetFloatId(Ihandle* ih, const char* name, int id, float value);
void  IupSetDoubleId(Ihandle* ih, const char* name, int id, double value);
void  IupSetRGBId(Ihandle *ih, const char* name, int id, unsigned char r, unsigned char g, unsigned char b);

char*  IupGetAttributeId(Ihandle *ih, const char* name, int id);
int    IupGetIntId(Ihandle *ih, const char* name, int id);
float  IupGetFloatId(Ihandle *ih, const char* name, int id);
double IupGetDoubleId(Ihandle *ih, const char* name, int id);
void   IupGetRGBId(Ihandle *ih, const char* name, int id, unsigned char *r, unsigned char *g, unsigned char *b);

void  IupSetAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* value);
void  IupSetStrAttributeId2(Ihandle* ih, const char* name, int lin, int col, const char* value);
void  IupSetStrfId2(Ihandle* ih, const char* name, int lin, int col, const char* format, ...);
void  IupSetIntId2(Ihandle* ih, const char* name, int lin, int col, int value);
void  IupSetFloatId2(Ihandle* ih, const char* name, int lin, int col, float value);
void  IupSetDoubleId2(Ihandle* ih, const char* name, int lin, int col, double value);
void  IupSetRGBId2(Ihandle *ih, const char* name, int lin, int col, unsigned char r, unsigned char g, unsigned char b);

char*  IupGetAttributeId2(Ihandle* ih, const char* name, int lin, int col);
int    IupGetIntId2(Ihandle* ih, const char* name, int lin, int col);
float  IupGetFloatId2(Ihandle* ih, const char* name, int lin, int col);
double IupGetDoubleId2(Ihandle* ih, const char* name, int lin, int col);
void   IupGetRGBId2(Ihandle *ih, const char* name, int lin, int col, unsigned char *r, unsigned char *g, unsigned char *b);

void      IupSetGlobal(const char* name, const char* value);
void      IupSetStrGlobal(const char* name, const char* value);
char*     IupGetGlobal(const char* name);

Ihandle*  IupSetFocus(Ihandle* ih);
Ihandle*  IupGetFocus(void);
Ihandle*  IupPreviousField(Ihandle* ih);
Ihandle*  IupNextField(Ihandle* ih);

Icallback IupGetCallback(Ihandle* ih, const char *name);
Icallback IupSetCallback(Ihandle* ih, const char *name, Icallback func);
Ihandle*  IupSetCallbacks(Ihandle* ih, const char *name, Icallback func, ...);

Icallback IupGetFunction(const char *name);
Icallback IupSetFunction(const char *name, Icallback func);

Ihandle*  IupGetHandle(const char *name);
Ihandle*  IupSetHandle(const char *name, Ihandle* ih);
int       IupGetAllNames(char** names, int n);
int       IupGetAllDialogs(char** names, int n);
char*     IupGetName(Ihandle* ih);

void      IupSetAttributeHandle(Ihandle* ih, const char* name, Ihandle* ih_named);
Ihandle*  IupGetAttributeHandle(Ihandle* ih, const char* name);

char*     IupGetClassName(Ihandle* ih);
char*     IupGetClassType(Ihandle* ih);
int       IupGetAllClasses(char** names, int n);
int       IupGetClassAttributes(const char* classname, char** names, int n);
int       IupGetClassCallbacks(const char* classname, char** names, int n);
void      IupSaveClassAttributes(Ihandle* ih);
void      IupCopyClassAttributes(Ihandle* src_ih, Ihandle* dst_ih);
void      IupSetClassDefaultAttribute(const char* classname, const char *name, const char* value);
int       IupClassMatch(Ihandle* ih, const char* classname);

Ihandle*  IupCreate(const char *classname);
Ihandle*  IupCreatev(const char *classname, void* *params);
Ihandle*  IupCreatep(const char *classname, void *first, ...);

Ihandle*  IupFill(void);
Ihandle*  IupRadio(Ihandle* child);
Ihandle*  IupVbox(Ihandle* child, ...);
Ihandle*  IupVboxv(Ihandle* *children);
Ihandle*  IupZbox(Ihandle* child, ...);
Ihandle*  IupZboxv(Ihandle* *children);
Ihandle*  IupHbox(Ihandle* child, ...);
Ihandle*  IupHboxv(Ihandle* *children);

Ihandle*  IupNormalizer(Ihandle* ih_first, ...);
Ihandle*  IupNormalizerv(Ihandle* *ih_list);

Ihandle*  IupCbox(Ihandle* child, ...);
Ihandle*  IupCboxv(Ihandle* *children);
Ihandle*  IupSbox(Ihandle *child);
Ihandle*  IupSplit(Ihandle* child1, Ihandle* child2);
Ihandle*  IupScrollBox(Ihandle* child);
Ihandle*  IupGridBox(Ihandle* child, ...);
Ihandle*  IupGridBoxv(Ihandle **children);
Ihandle*  IupExpander(Ihandle *child);
Ihandle*  IupDetachBox(Ihandle *child);
Ihandle*  IupBackgroundBox(Ihandle *child);

Ihandle*  IupFrame(Ihandle* child);

Ihandle*  IupImage(int width, int height, const unsigned char *pixmap);
Ihandle*  IupImageRGB(int width, int height, const unsigned char *pixmap);
Ihandle*  IupImageRGBA(int width, int height, const unsigned char *pixmap);

Ihandle*  IupItem(const char* title, const char* action);
Ihandle*  IupSubmenu(const char* title, Ihandle* child);
Ihandle*  IupSeparator(void);
Ihandle*  IupMenu(Ihandle* child, ...);
Ihandle*  IupMenuv(Ihandle* *children);

Ihandle*  IupButton(const char* title, const char* action);
Ihandle*  IupCanvas(const char* action);
Ihandle*  IupDialog(Ihandle* child);
Ihandle*  IupUser(void);
Ihandle*  IupLabel(const char* title);
Ihandle*  IupList(const char* action);
Ihandle*  IupText(const char* action);
Ihandle*  IupMultiLine(const char* action);
Ihandle*  IupToggle(const char* title, const char* action);
Ihandle*  IupTimer(void);
Ihandle*  IupClipboard(void);
Ihandle*  IupProgressBar(void);
Ihandle*  IupVal(const char *type);
Ihandle*  IupTabs(Ihandle* child, ...);
Ihandle*  IupTabsv(Ihandle* *children);
Ihandle*  IupTree(void);
Ihandle*  IupLink(const char* url, const char* title);
Ihandle*  IupFlatButton(const char* title);

Ihandle*  IupSpin(void);
Ihandle*  IupSpinbox(Ihandle* child);

int IupSaveImageAsText(Ihandle* ih, const char* file_name, const char* format, const char* name);

void  IupTextConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos);
void  IupTextConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col);

int   IupConvertXYToPos(Ihandle* ih, int x, int y);

int   IupTreeSetUserId(Ihandle* ih, int id, void* userid);
void* IupTreeGetUserId(Ihandle* ih, int id);
int   IupTreeGetId(Ihandle* ih, void *userid);
void  IupTreeSetAttributeHandle(Ihandle* ih, const char* name, int id, Ihandle* ih_named);

Ihandle* IupFileDlg(void);
Ihandle* IupMessageDlg(void);
Ihandle* IupColorDlg(void);
Ihandle* IupFontDlg(void);
Ihandle* IupProgressDlg(void);

int  IupGetFile(char *arq);
void IupMessage(const char *title, const char *msg);
void IupMessagef(const char *title, const char *format, ...);
int  IupAlarm(const char *title, const char *msg, const char *b1, const char *b2, const char *b3);
int  IupScanf(const char *format, ...);
int  IupListDialog(int type, const char *title, int size, const char** list,
                   int op, int max_col, int max_lin, int* marks);
int  IupGetText(const char* title, char* text);
int  IupGetColor(int x, int y, unsigned char* r, unsigned char* g, unsigned char* b);

typedef int(*Iparamcb)(Ihandle* dialog, int param_index, void* user_data);
int IupGetParam(const char* title, Iparamcb action, void* user_data, const char* format, ...);
int IupGetParamv(const char* title, Iparamcb action, void* user_data, const char* format, int param_count, int param_extra, void** param_data);
Ihandle* IupParamf(const char* format);
Ihandle* IupParamBox(Ihandle* parent, Ihandle** params, int count);

Ihandle* IupLayoutDialog(Ihandle* dialog);
Ihandle* IupElementPropertiesDialog(Ihandle* elem);
#endif

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

    void SetUserData(const char* name, void* data)
    {
      IupSetAttribute(ih, name, (char*)data);
    }
    void* GetUserData(const char* name)
    {
      return (void*)IupGetAttribute(ih, name);
    }

  };
  

  class Image : public Handle
  {
  public:
    Image(const char* filename) : Handle(IupLoadImage(filename)) {}
    Image(const im::Image& image) : Handle(IupImageFromImImage(image.GetHandle())) {}

    int Save(const char* filename, const char* format)
    {
      return IupSaveImage(ih, filename, format);
    }
  };
  class Clipboard : public Handle
  {
  public:
    Clipboard() : Handle(IupClipboard()) {}

    void SetImage(const im::Image& image)
    {
      SetUserData("NATIVEIMAGE", IupGetImageNativeHandle(image.GetHandle()));
    }

    im::Image GetImage(void)
    {
      return im::Image(IupGetNativeHandleImage(GetUserData("NATIVEIMAGE")));
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
  class MglPlot : public Handle
  {
  public:
    MglPlot() : Handle(IupMglPlot()) {}

    static void Open() { IupMglPlotOpen(); }

    void Begin(int dim)
    {
      IupMglPlotBegin(ih, dim);
    }
    void Add1D(const char* name, double y)
    {
      IupMglPlotAdd1D(ih, name, y);
    }
    void Add2D(double x, double y)
    {
      IupMglPlotAdd2D(ih, x, y);
    }
    void Add3D(double x, double y, double z)
    {
      IupMglPlotAdd3D(ih, x, y, z);
    }
    int End()
    {
      return IupMglPlotEnd(ih);
    }

    int NewDataSet(int dim)
    {
      return IupMglPlotNewDataSet(ih, dim);
    }

    void Insert1D(int ds_index, int sample_index, const char** names, const double* y, int count)
    {
      IupMglPlotInsert1D(ih, ds_index, sample_index, names, y, count);
    }
    void Insert2D(int ds_index, int sample_index, const double* x, const double* y, int count)
    {
      IupMglPlotInsert2D(ih, ds_index, sample_index, x, y, count);
    }
    void Insert3D(int ds_index, int sample_index, const double* x, const double* y, const double* z, int count)
    {
      IupMglPlotInsert3D(ih, ds_index, sample_index, x, y, z, count);
    }

    void Set1D(int ds_index, const char** names, const double* y, int count)
    {
      IupMglPlotSet1D(ih, ds_index, names, y, count);
    }
    void Set2D(int ds_index, const double* x, const double* y, int count)
    {
      IupMglPlotSet2D(ih, ds_index, x, y, count);
    }
    void Set3D(int ds_index, const double* x, const double* y, const double* z, int count)
    {
      IupMglPlotSet3D(ih, ds_index, x, y, z, count);
    }
    void SetFormula(int ds_index, const char* formulaX, const char* formulaY, const char* formulaZ, int count)
    {
      IupMglPlotSetFormula(ih, ds_index, formulaX, formulaY, formulaZ, count);
    }

    void SetData(int ds_index, const double* data, int count_x, int count_y, int count_z)
    {
      IupMglPlotSetData(ih, ds_index, data, count_x, count_y, count_z);
    }
    void LoadData(int ds_index, const char* filename, int count_x, int count_y, int count_z)
    {
      IupMglPlotLoadData(ih, ds_index, filename, count_x, count_y, count_z);
    }
    void SetFromFormula(int ds_index, const char* formula, int count_x, int count_y, int count_z)
    {
      IupMglPlotSetFromFormula(ih, ds_index, formula, count_x, count_y, count_z);
    }

    void Transform(double x, double y, double z, int &ix, int &iy)
    {
      IupMglPlotTransform(ih, x, y, z, &ix, &iy);
    }
    void TransformTo(int ix, int iy, double &x, double &y, double &z)
    {
      IupMglPlotTransformTo(ih, ix, iy, &x, &y, &z);
    }

    void DrawMark(double x, double y, double z)
    {
      IupMglPlotDrawMark(ih, x, y, z);
    }
    void DrawLine(double x1, double y1, double z1, double x2, double y2, double z2)
    {
      IupMglPlotDrawLine(ih, x1, y1, z1, x2, y2, z2);
    }
    void DrawText(const char* text, double x, double y, double z)
    {
      IupMglPlotDrawText(ih, text, x, y, z);
    }

    void PaintTo(const char* format, int w, int h, double dpi, unsigned char* data)
    {
      IupMglPlotPaintTo(ih, format, w, h, dpi, (void*)data);
    }
    void PaintTo(const char* format, int w, int h, double dpi, const char* filename)
    {
      IupMglPlotPaintTo(ih, format, w, h, dpi, (void*)filename);
    }

  };
  class MglLabel : public Handle
  {
  public:
    MglLabel(const char* title) : Handle(IupMglLabel(title)) {}
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
