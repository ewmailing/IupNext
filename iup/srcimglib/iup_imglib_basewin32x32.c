/** \file
 * \brief IupImgLib
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>

#include "iup.h"
#include "iup_image.h"

#include "iup_imglib.h"

/* source code, included only here */
#include "iup_imglib_basewin32x32.h"

void iupImglibBaseLibWin32x32Open(void)
{
  iupImageStockSet("IUP_ActionCancel32", load_image_ActionCancel, 0);
  iupImageStockSet("IUP_ActionOk32", load_image_ActionOk, 0);
  iupImageStockSet("IUP_ArrowDown32", load_image_ArrowDown, 0);
  iupImageStockSet("IUP_ArrowLeft32", load_image_ArrowLeft, 0);
  iupImageStockSet("IUP_ArrowRight32", load_image_ArrowRight, 0);
  iupImageStockSet("IUP_ArrowUp32", load_image_ArrowUp, 0);
  iupImageStockSet("IUP_EditCopy32", load_image_EditCopy, 0);
  iupImageStockSet("IUP_EditCut32", load_image_EditCut, 0);
  iupImageStockSet("IUP_EditErase32", load_image_EditErase, 0);
  iupImageStockSet("IUP_EditFind32", load_image_EditFind, 0);
  iupImageStockSet("IUP_EditPaste32", load_image_EditPaste, 0);
  iupImageStockSet("IUP_EditRedo32", load_image_EditRedo, 0);
  iupImageStockSet("IUP_EditUndo32", load_image_EditUndo, 0);
  iupImageStockSet("IUP_FileClose32", load_image_FileClose, 0);
  iupImageStockSet("IUP_FileCloseAll32", load_image_FileCloseAll, 0);
  iupImageStockSet("IUP_FileNew32", load_image_FileNew, 0);
  iupImageStockSet("IUP_FileOpen32", load_image_FileOpen, 0);
  iupImageStockSet("IUP_FileProperties32", load_image_FileProperties, 0);
  iupImageStockSet("IUP_FileSave32", load_image_FileSave, 0);
  iupImageStockSet("IUP_FileSaveAll32", load_image_FileSaveAll, 0);
  iupImageStockSet("IUP_FileText32", load_image_FileText, 0);
  iupImageStockSet("IUP_FontBold32", load_image_FontBold, 0);
  iupImageStockSet("IUP_FontDialog32", load_image_FontDialog, 0);
  iupImageStockSet("IUP_FontItalic32", load_image_FontItalic, 0);
  iupImageStockSet("IUP_MediaForward32", load_image_MediaForward, 0);
  iupImageStockSet("IUP_MediaGoToBegin32", load_image_MediaGoToBegin, 0);
  iupImageStockSet("IUP_MediaGoToEnd32", load_image_MediaGoToEnd, 0);
  iupImageStockSet("IUP_MediaPause32", load_image_MediaPause, 0);
  iupImageStockSet("IUP_MediaPlay32", load_image_MediaPlay, 0);
  iupImageStockSet("IUP_MediaRecord32", load_image_MediaRecord, 0);
  iupImageStockSet("IUP_MediaReverse32", load_image_MediaReverse, 0);
  iupImageStockSet("IUP_MediaRewind32", load_image_MediaRewind, 0);
  iupImageStockSet("IUP_MediaStop32", load_image_MediaStop, 0);
  iupImageStockSet("IUP_MessageError32", load_image_MessageError, 0);
  iupImageStockSet("IUP_MessageHelp32", load_image_MessageHelp, 0);
  iupImageStockSet("IUP_MessageInfo32", load_image_MessageInfo, 0);
  iupImageStockSet("IUP_NavigateHome32", load_image_NavigateHome, 0);
  iupImageStockSet("IUP_NavigateRefresh32", load_image_NavigateRefresh, 0);
  iupImageStockSet("IUP_Print32", load_image_Print, 0);
  iupImageStockSet("IUP_PrintPreview32", load_image_PrintPreview, 0);
  iupImageStockSet("IUP_ToolsColor32", load_image_ToolsColor, 0);
  iupImageStockSet("IUP_ToolsSettings32", load_image_ToolsSettings, 0);
  iupImageStockSet("IUP_ToolsSortAscend32", load_image_ToolsSortAscend, 0);
  iupImageStockSet("IUP_ToolsSortDescend32", load_image_ToolsSortDescend, 0);
  iupImageStockSet("IUP_ViewFullScreen32", load_image_ViewFullScreen, 0);
  iupImageStockSet("IUP_Webcam32", load_image_Webcam, 0);
  iupImageStockSet("IUP_WindowsCascade32", load_image_WindowsCascade, 0);
  iupImageStockSet("IUP_WindowsTile32", load_image_WindowsTile, 0);
  iupImageStockSet("IUP_Zoom32", load_image_Zoom, 0);
  iupImageStockSet("IUP_ZoomActualSize32", load_image_ZoomActualSize, 0);
  iupImageStockSet("IUP_ZoomIn32", load_image_ZoomIn, 0);
  iupImageStockSet("IUP_ZoomOut32", load_image_ZoomOut, 0);
  iupImageStockSet("IUP_ZoomSelection32", load_image_ZoomSelection, 0);
}

