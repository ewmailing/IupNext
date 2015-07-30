
/* avoid multiple inclusions */
#ifndef SIMPLE_PAINT_TOOLBOX_H
#define SIMPLE_PAINT_TOOLBOX_H

#include <stdlib.h>
#include <iup.h>
#include <cd.h>

#include <iup_class_cbs.hpp>



class SimplePaintToolbox
{
public:
  enum Tool {
    TOOL_POINTER, TOOL_COLORPICKER,
    TOOL_PENCIL, TOOL_LINE,
    TOOL_RECT, TOOL_BOX,
    TOOL_ELLIPSE, TOOL_OVAL,
    TOOL_TEXT, TOOL_FILLCOLOR
  };

private:
  Ihandle *toolbox, *config, *paint_canvas;

  Tool tool_index;

  struct
  {
    long color;
    int line_width;
    int line_style;
    double fill_tol;
  } options;

public:
  SimplePaintToolbox()
    :toolbox(NULL), config(NULL), paint_canvas(NULL), tool_index(TOOL_POINTER)
  {
    options.color = CD_BLACK;
    options.line_width = 1;
    options.line_style = 0;
    options.fill_tol = 50;
  }

  Tool ToolIndex() const { return tool_index; }

  long Color() const { return options.color; }
  int LineWidth() const { return options.line_width; }
  int LineStyle() const { return options.line_style; }
  double FillTol() const { return options.fill_tol; }
  const char* Font() const { return IupGetAttribute(toolbox, "TOOLFONT"); }
  const char* Text() const { return IupGetAttribute(toolbox, "TOOLTEXT"); }

  void SetColor(long new_color);

  void MoveDialog(int dx, int dy);
  bool HideDialog();
  void ShowDialog();
  void CreateDialog(Ihandle* canvas, Ihandle* config);

  void ToolGetText();

protected:

  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, CloseCallback);
  IUP_CLASS_DECLARECALLBACK_IFni(SimplePaintToolbox, ToolActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, ToolColorActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, ToolWidthValueChangedCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, ToolStyleValueChangedCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, ToolFontActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaintToolbox, ToolFillTolValueChangedCallback);
};


#endif
