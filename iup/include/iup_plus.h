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


#if 0 //TODO

Ihandle*  IupAppend(Ihandle* ih, Ihandle* child);
Ihandle*  IupInsert(Ihandle* ih, Ihandle* ref_child, Ihandle* child);
Ihandle*  IupGetChild(Ihandle* ih, int pos);
int       IupGetChildPos(Ihandle* ih, Ihandle* child);
int       IupGetChildCount(Ihandle* ih);

int       IupPopup(Ihandle* ih, int x, int y);
int       IupShow(Ihandle* ih);
int       IupShowXY(Ihandle* ih, int x, int y);
int       IupHide(Ihandle* ih);

===============================================================================

void      IupResetAttribute(Ihandle *ih, const char* name);
int       IupGetAllAttributes(Ihandle* ih, char** names, int n);

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

===============================================================================

Icallback IupGetCallback(Ihandle* ih, const char *name);
Icallback IupSetCallback(Ihandle* ih, const char *name, Icallback func);
Ihandle*  IupSetCallbacks(Ihandle* ih, const char *name, Icallback func, ...);

void      IupSetAttributeHandle(Ihandle* ih, const char* name, Ihandle* ih_named);
Ihandle*  IupGetAttributeHandle(Ihandle* ih, const char* name);

===============================================================================

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
Ihandle*  IupMenu(Ihandle* child, ...);
Ihandle*  IupMenuv(Ihandle* *children);
Ihandle*  IupDialog(Ihandle* child);
Ihandle*  IupFrame(Ihandle* child);
Ihandle*  IupSubmenu(const char* title, Ihandle* child);
Ihandle*  IupTabs(Ihandle* child, ...);
Ihandle*  IupTabsv(Ihandle* *children);
Ihandle*  IupSpinbox(Ihandle* child);

===============================================================================


void  IupTextConvertLinColToPos(Ihandle* ih, int lin, int col, int *pos);
void  IupTextConvertPosToLinCol(Ihandle* ih, int pos, int *lin, int *col);

int   IupConvertXYToPos(Ihandle* ih, int x, int y);

int   IupTreeSetUserId(Ihandle* ih, int id, void* userid);
void* IupTreeGetUserId(Ihandle* ih, int id);
int   IupTreeGetId(Ihandle* ih, void *userid);
void  IupTreeSetAttributeHandle(Ihandle* ih, const char* name, int id, Ihandle* ih_named);

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

  inline int Open(int &argc, char **&argv)
  {
    return IupOpen(&argc, &argv);
  }
  inline void Close()
  {
    IupClose();
  }
  inline void ImageLibOpen()
  {
    IupImageLibOpen();
  }

  inline int MainLoop()
  {
    return IupMainLoop();
  }
  inline int LoopStep()
  {
    return IupLoopStep();
  }
  inline int LoopStepWait()
  {
    return IupLoopStepWait();
  }
  inline int MainLoopLevel()
  {
    return IupMainLoopLevel();
  }
  inline void Flush()
  {
    IupFlush();
  }
  inline void ExitLoop()
  {
    IupExitLoop();
  }

  inline int RecordInput(const char* filename, int mode)
  {
    return IupRecordInput(filename, mode);
  }
  inline int  PlayInput(const char* filename)
  {
    return IupPlayInput(filename);
  }

  inline int Help(const char* url)
  {
    return IupHelp(url);
  }
  inline const char* Load(const char *filename)
  {
    return IupLoad(filename);
  }
  inline const char* LoadBuffer(const char *buffer)
  {
    return IupLoadBuffer(buffer);
  }

  inline void SetLanguage(const char *lng)
  {
    IupSetLanguage(lng);
  }
  inline const char* GetLanguage()
  {
    return IupGetLanguage();
  }
  inline void SetLanguageString(const char* name, const char* str)
  {
    IupSetLanguageString(name, str);
  }
  inline void StoreLanguageString(const char* name, const char* str)
  {
    IupStoreLanguageString(name, str);
  }
  inline const char* GetLanguageString(const char* name)
  {
    return IupGetLanguageString(name);
  }

  inline int GetAllClasses(char** names, int n)
  {
    return IupGetAllClasses(names, n);
  }
  inline int GetClassAttributes(const char* classname, char** names, int n)
  {
    return IupGetClassAttributes(classname, names, n);
  }
  inline int GetClassCallbacks(const char* classname, char** names, int n)
  {
    return IupGetClassCallbacks(classname, names, n);
  }
  inline void SetClassDefaultAttribute(const char* classname, const char *name, const char* value)
  {
    IupSetClassDefaultAttribute(classname, name, value);
  }

  inline void SetGlobal(const char* name, const char* value)
  {
    IupSetGlobal(name, value);
  }
  inline void SetStringGlobal(const char* name, const char* value)
  {
    IupSetStrGlobal(name, value);
  }
  inline char* GetGlobal(const char* name)
  {
    return IupGetGlobal(name);
  }


  inline int GetFile(char *arq)
  {
    return IupGetFile(arq);
  }
  inline void Message(const char *title, const char *msg)
  {
    IupMessage(title, msg);
  }
  //TODO
  //void Messagef(const char *title, const char *format, ...)
  //{
  //  IupMessagef(title, format, ...);
  //}
  inline int Alarm(const char *title, const char *msg, const char *b1, const char *b2, const char *b3)
  {
    return IupAlarm(title, msg, b1, b2, b3);
  }
  //TODO
  //int Scanf(const char *format, ...)
  //{
  //  return IupScanf(format, ...);
  //}
  inline int ListDialog(int type, const char *title, int size, const char** list, int op, int max_col, int max_lin, int* marks)
  {
    return IupListDialog(type, title, size, list, op, max_col, max_lin, marks);
  }
  inline int GetText(const char* title, char* text)
  {
    return IupGetText(title, text);
  }
  inline int GetColor(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b)
  {
    return IupGetColor(x, y, &r, &g, &b);
  }

  //TODO
  //STL std::string com ifdef

  inline int GetAllNames(char** names, int n)
  {
    return IupGetAllNames(names, n);
  }
  inline int GetAllDialogs(char** names, int n)
  {
    return IupGetAllDialogs(names, n);
  }


  class Handle
  {
  protected:
    Ihandle* ih;

    /* forbidden */
    Handle() { ih = 0; };

  public:
    Handle(Ihandle* ref_ih)
    {
      ih = ref_ih;
    }
    // There is no destructor because all iup::Handle are just a reference to the Ihandle*,
    // since several IUP elements are automatically destroyed when the dialog is destroyed
    // So, to force an element to be destructed call the Destroy method

    Ihandle* GetHandle() const { return ih; }

    void SetUserData(const char* name, void* data)
    {
      IupSetAttribute(ih, name, (char*)data);
    }
    void* GetUserData(const char* name)
    {
      return (void*)IupGetAttribute(ih, name);
    }
    void SetInteger(const char* name, int value)
    {
      IupSetInt(ih, name, value);
    }
    int GetInteger(const char* name)
    {
      return IupGetInt(ih, name);
    }


    void Update()
    {
      IupUpdate(ih);
    }
    void UpdateChildren()
    {
      IupUpdateChildren(ih);
    }
    void Redraw(bool children)
    {
      IupRedraw(ih, children ? 1 : 0);
    }
    void Refresh()
    {
      IupRefresh(ih);
    }
    void RefreshChildren()
    {
      IupRefreshChildren(ih);
    }

    void Destroy()
    {
      IupDestroy(ih);
    }
    void Detach(const Handle& child)
    {
      IupDetach(child.ih);
    }

    Handle GetNextChild(const Handle& child)
    {
      return Handle(IupGetNextChild(ih, child.ih));
    }
    Handle GetBrother()
    {
      return Handle(IupGetBrother(ih));
    }
    Handle GetParent()
    {
      return Handle(IupGetParent(ih));
    }
    Handle GetDialog()
    {
      return Handle(IupGetDialog(ih));
    }
    Handle GetDialogChild(const char* name)
    {
      return Handle(IupGetDialogChild(ih, name));
    }
    int Reparent(const Handle& new_parent, const Handle& ref_child)
    {
      return IupReparent(ih, new_parent.ih, ref_child.ih);
    }

    int Map()
    {
      return IupMap(ih);
    }
    void Unmap()
    {
      IupUnmap(ih);
    }

    Handle SetFocus()
    {
      return Handle(IupSetFocus(ih));
    }
    Handle PreviousField()
    {
      return Handle(IupPreviousField(ih));
    }
    Handle NextField()
    {
      return Handle(IupNextField(ih));
    }

    char* GetName()
    {
      return IupGetName(ih);
    }

    char* GetClassName()
    {
      return IupGetClassName(ih);
    }
    char* GetClassType()
    {
      return IupGetClassType(ih);
    }
    void SaveClassAttributes()
    {
      IupSaveClassAttributes(ih);
    }
    void CopyClassAttributesTo(const Handle& dst)
    {
      IupCopyClassAttributes(ih, dst.ih);
    }
    int ClassMatch(const char* classname)
    {
      return IupClassMatch(ih, classname);
    }

  };

  inline Handle GetFocus()
  {
    return Handle(IupGetFocus());
  }
  inline Handle GetHandle(const char *name)
  {
    return Handle(IupGetHandle(name));
  }
  inline Handle SetHandle(const char *name, const Handle& handle)
  {
    return Handle(IupSetHandle(name, handle.GetHandle()));
  }
  inline void  SetLanguagePack(const Handle& handle)
  {
    IupSetLanguagePack(handle.GetHandle());
  }

  class Container : public Handle
  {
  public:
    Container(Ihandle* _ih) : Handle(_ih) {}
  };
  class Dialog : public Container
  {
  public:
    Dialog(Ihandle* _ih) : Container(_ih) {}
  };
  class Control : public Handle
  {
  public:
    Control(Ihandle* _ih) : Handle(_ih) {}
  };
  class MenuControl : public Handle
  {
  public:
    MenuControl(Ihandle* _ih) : Handle(_ih) {}
  };

  class Image : public Handle
  {
  public:
    Image(const char* filename) : Handle(IupLoadImage(filename)) {}
    Image(const im::Image& image) : Handle(IupImageFromImImage(image.GetHandle())) {}

    int Save(const char* filename, const char* im_format)
    {
      return IupSaveImage(ih, filename, im_format);
    }
    int SaveAsText(const char* filename, const char* iup_format, const char* name)
    {
      return IupSaveImageAsText(ih, filename, iup_format, name);
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
  class User : public Handle
  {
  public:
    User() : Handle(IupUser()) {}
  };
  class Timer : public Handle
  {
  public:
    Timer() : Handle(IupTimer()) {}
  };
  class MenuSeparator : public MenuControl
  {
  public:
    MenuSeparator() : MenuControl(IupSeparator()) {}
  };
  class MenuItem : public MenuControl
  {
  public:
    MenuItem(const char* title = 0) : MenuControl(IupItem(title, 0)) {}
  };
  class Canvas : public Control
  {
  public:
    Canvas() : Control(IupCanvas(0)) {}
  };
  class Link : public Control
  {
  public:
    Link(const char* url = 0, const char* title = 0) : Control(IupLink(url, title)) {}
  };
  class Label : public Control
  {
  public:
    Label(const char* title = 0) : Control(IupLabel(title)) {}
  };
  class Button : public Control
  {
  public:
    Button(const char* title = 0) : Control(IupButton(title, 0)) {}
  };
  class FlatButton : public Control
  {
  public:
    FlatButton(const char* title = 0) : Control(IupFlatButton(title)) {}
  };
  class Toggle : public Control
  {
  public:
    Toggle(const char* title = 0) : Control(IupToggle(title, 0)) {}
  };
  class Fill: public Control
  {
  public:
    Fill() : Control(IupFill()) {}
  };
  class Spin: public Control
  {
  public:
    Spin() : Control(IupSpin()) {}
  };
  class Tree: public Control
  {
  public:
    Tree() : Control(IupTree()) {}
  };
  class Val : public Control
  {
  public:
    Val(const char* orientation = 0) : Control(IupVal(orientation)) {}
  };
  class ProgressBar: public Control
  {
  public:
    ProgressBar() : Control(IupProgressBar()) {}
  };
  class List: public Control
  {
  public:
    List() : Control(IupList(0)) {}
  };
  class Text: public Control
  {
  public:
    Text() : Control(IupText(0)) {}
  };

  class FileDialog : public Dialog
  {
  public:
    FileDialog() : Dialog(IupFileDlg()) {}
  };
  class MessageDialog : public Dialog
  {
  public:
    MessageDialog() : Dialog(IupMessageDlg()) {}
  };
  class ColorDialog : public Dialog
  {
  public:
    ColorDialog() : Dialog(IupColorDlg()) {}
  };
  class FontDialog : public Dialog
  {
  public:
    FontDialog() : Dialog(IupFontDlg()) {}
  };
  class ProgressDialog : public Dialog
  {
  public:
    ProgressDialog() : Dialog(IupProgressDlg()) {}
  };
  class GLCanvas : public Control
  {
  public:
    GLCanvas() : Control(IupGLCanvas(0)) {}

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
  class Dial : public Control
  {
  public:
    Dial(const char* orientation = 0) : Control(IupDial(orientation)) {}
  };
  class Gauge : public Control
  {
  public:
    Gauge() : Control(IupGauge()) {}
  };
  class ColorBrowser : public Control
  {
  public:
    ColorBrowser() : Control(IupColorBrowser()) {}
  };
  class Cells : public Control
  {
  public:
    Cells() : Control(IupCells()) {}
  };
  class Colorbar : public Control
  {
  public:
    Colorbar() : Control(IupColorbar()) {}
  };
  class Matrix : public Control
  {
  public:
    Matrix() : Control(IupMatrix(0)) {}

    void SetFormula(int col, const char* formula, const char* init = 0)
    {
      IupMatrixSetFormula(ih, col, formula, init);
    }
    void SetDynamic(Ihandle* ih, const char* init = 0)
    {
      IupMatrixSetDynamic(ih, init);
    }
  };
  class MatrixList : public Control
  {
  public:
    MatrixList() : Control(IupMatrixList()) {}
  };
  class MatrixEx : public Control
  {
  public:
    MatrixEx() : Control(IupMatrixEx()) {}

    static void Open() { IupMatrixExOpen(); }
  };
  class GLControls
  {
  public:
    static void Open() { IupGLControlsOpen(); }
  };
  class GLSubCanvas : public Control
  {
  public:
    GLSubCanvas() : Control(IupGLSubCanvas()) {}
  };
  class GLSeparator : public Control
  {
  public:
    GLSeparator() : Control(IupGLSeparator()) {}
  };
  class GLProgressBar : public Control
  {
  public:
    GLProgressBar() : Control(IupGLProgressBar()) {}
  };
  class GLVal : public Control
  {
  public:
    GLVal() : Control(IupGLVal()) {}
  };
  class GLLabel : public Control
  {
  public:
    GLLabel(const char* title = 0) : Control(IupGLLabel(title)) {}
  };
  class GLButton : public Control
  {
  public:
    GLButton(const char* title = 0) : Control(IupGLButton(title)) {}
  };
  class GLToggle : public Control
  {
  public:
    GLToggle(const char* title = 0) : Control(IupGLToggle(title)) {}
  };
  class GLLink : public Control
  {
  public:
    GLLink(const char *url = 0, const char* title = 0) : Control(IupGLLink(url, title)) {}
  };
  class GLFrame : public Control
  {
  public:
    GLFrame(Handle& child) : Control(IupGLFrame(child.GetHandle())) {}
    GLFrame() : Control(IupGLFrame(0)) {}
  };
  class GLExpander : public Control
  {
  public:
    GLExpander(Handle& child) : Control(IupGLExpander(child.GetHandle())) {}
    GLExpander() : Control(IupGLExpander(0)) {}
  };
  class GLScrollBox : public Control
  {
  public:
    GLScrollBox(Handle& child) : Control(IupGLScrollBox(child.GetHandle())) {}
    GLScrollBox() : Control(IupGLScrollBox(0)) {}
  };
  class GLSizeBox : public Control
  {
  public:
    GLSizeBox(Handle& child) : Control(IupGLSizeBox(child.GetHandle())) {}
    GLSizeBox() : Control(IupGLSizeBox(0)) {}
  };
  class GLCanvasBox : public Container
  {
  public:
    GLCanvasBox() : Container(IupGLCanvasBox(0)) {}
    GLCanvasBox(Handle& child) : Container(IupGLCanvasBox(child.GetHandle(), 0)) {}
    GLCanvasBox(Handle* child, int count) : Container(IupGLCanvasBox(0))
    {
      for (int i = 0; i < count; i++)
        IupAppend(ih, child[i].GetHandle());
    }
  };
  class Plot : public Control
  {
  public:
    Plot() : Control(IupPlot()) {}

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
  class MglPlot : public Control
  {
  public:
    MglPlot() : Control(IupMglPlot()) {}

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
  class MglLabel : public Control
  {
  public:
    MglLabel(const char* title) : Control(IupMglLabel(title)) {}
  };
  class OleControl : public Control
  {
  public:
    OleControl(const char* progid) : Control(IupOleControl(progid)) {}

    static void Open() { IupOleControlOpen(); }
  };
  class WebBrowser : public Control
  {
  public:
    WebBrowser() : Control(IupWebBrowser()) {}

    static void Open() { IupWebBrowserOpen(); }
  };
  class Scintilla : public Control
  {
  public:
    Scintilla(): Control(IupScintilla()) {}

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
