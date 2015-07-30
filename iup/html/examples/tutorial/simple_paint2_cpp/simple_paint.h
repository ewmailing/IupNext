
/* avoid multiple inclusions */
#ifndef SIMPLE_PAINT_H
#define SIMPLE_PAINT_H

#include "simple_paint_file.h"
#include "simple_paint_toolbox.h"



class SimplePaint
{
  Ihandle *dlg, *config, *canvas;

  cdCanvas* cd_canvas;
  SimplePaintFile file;
  SimplePaintToolbox toolbox;

  struct 
  {
    bool overlay;
    int start_x, start_y;
    int end_x, end_y;
    int start_cursor_x, start_cursor_y;
  } interact;

public:

  SimplePaint();

  void CheckNewFile();

  void OpenFile(const char* filename);

protected:

  void CreateMainDialog();
  Ihandle* CreateStatusbar();
  Ihandle* CreateToolbar();
  Ihandle* CreateMainMenu();

  void DrawToolOverlay(cdCanvas* cnv, int start_x, int start_y, int end_x, int end_y);
  void DrawPencil(int start_x, int start_y, int end_x, int end_y);

  void SelectFile(bool is_open);

  void UpdateFile();
  void UpdateImage(imImage* new_image, bool update_size = false);
  void UpdateZoom(double zoom_index);

  void ToggleBarVisibility(Ihandle* item, Ihandle* bar);

  double ViewZoomRect(int *_x, int *_y, int *_view_width, int *_view_height);

  IUP_CLASS_DECLARECALLBACK_IFnii(SimplePaint, DialogMoveCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ConfigRecentCallback);
  IUP_CLASS_DECLARECALLBACK_IFns(SimplePaint, DropfilesCallback);

  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemNewActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemOpenActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemSaveActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemSaveasActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemRevertActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemPagesetupActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemPrintActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemExitActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemCopyActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemPasteActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemZoominActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemZoomoutActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemActualsizeActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemZoomgridActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemBackgroundActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemToolbarActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemToolboxActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemHelpActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemAboutActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemStatusbarActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemResizeActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemMirrorActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemFlipActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemRotate180ActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemRotate90cwActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemRotate90ccwActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemNegativeActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ItemBrightcontActionCallback);

  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, FileMenuOpenCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, EditMenuOpenCallback);

  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, CanvasActionCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, CanvasMapCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, CanvasUnmapCallback);
  IUP_CLASS_DECLARECALLBACK_IFnfiis(SimplePaint, CanvasWheelCallback);
  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, CanvasResizeCallback);
  IUP_CLASS_DECLARECALLBACK_IFniis(SimplePaint, CanvasMotionCallback);
  IUP_CLASS_DECLARECALLBACK_IFniiii(SimplePaint, CanvasButtonCallback);

  IUP_CLASS_DECLARECALLBACK_IFn(SimplePaint, ZoomValueChangedCallback);
};

#endif
