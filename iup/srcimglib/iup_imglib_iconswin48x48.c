/** \file
 * \brief IupImgLib
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>

#include "iup.h"

#include "iup_str.h"
#include "iup_image.h"

#include "iup_imglib.h"

#ifdef IUP_IMGLIB_LARGE
/* source code, included only here */
#include "iup_imglib_iconswin48x48.h"

void iupImglibIconsWin48x48Open(void)
{
  iupImageStockSet("IUP_DeviceCamera", load_image_IUP_DeviceCamera, 0);
  iupImageStockSet("IUP_DeviceCD", load_image_IUP_DeviceCD, 0);
  iupImageStockSet("IUP_DeviceCellPhone", load_image_IUP_DeviceCellPhone, 0);
  iupImageStockSet("IUP_DeviceComputer", load_image_IUP_DeviceComputer, 0);
  iupImageStockSet("IUP_DeviceHardDrive", load_image_IUP_DeviceHardDrive, 0);
  iupImageStockSet("IUP_DeviceFax", load_image_IUP_DeviceFax, 0);
  iupImageStockSet("IUP_DeviceMP3", load_image_IUP_DeviceMP3, 0);
  iupImageStockSet("IUP_DeviceNetwork", load_image_IUP_DeviceNetwork, 0);
  iupImageStockSet("IUP_DevicePDA", load_image_IUP_DevicePDA, 0);
  iupImageStockSet("IUP_DevicePrinter", load_image_IUP_DevicePrinter, 0);
  iupImageStockSet("IUP_DeviceScanner", load_image_IUP_DeviceScanner, 0);
  iupImageStockSet("IUP_DeviceSound", load_image_IUP_DeviceSound, 0);
  iupImageStockSet("IUP_DeviceVideo", load_image_IUP_DeviceVideo, 0);

  iupImageStockSet("IUP_IconMessageError", load_image_IUP_IconMessageError, 0);
  iupImageStockSet("IUP_IconMessageHelp", load_image_IUP_IconMessageHelp, 0);
  iupImageStockSet("IUP_IconMessageInfo", load_image_IUP_IconMessageInfo, 0);
  iupImageStockSet("IUP_IconMessageSecurity", load_image_IUP_IconMessageSecurity, 0);
  iupImageStockSet("IUP_IconMessageWarning", load_image_IUP_IconMessageWarning, 0);
}

void iupImglibIconsGtk48x48Open(void)
{
  iupImageStockSet("IUP_IconMessageError", 0, "gtk-dialog-error");
  iupImageStockSet("IUP_IconMessageHelp", 0, "gtk-dialog-question");
  iupImageStockSet("IUP_IconMessageInfo", 0, "gtk-dialog-info");
  iupImageStockSet("IUP_IconMessageSecurity", 0, "gtk-dialog-authentication");
  iupImageStockSet("IUP_IconMessageWarning", 0, "gtk-dialog-warning");
}
#endif
