/** \file
 * \brief IupImgLib
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>

#include "iup.h"
#include "iup_image.h"

#include "iup_imglib.h"

void iupImglibIconsGtk348x48Open(void)
{
  iupImageStockSet("IUP_IconMessageError", load_image_dialog_error, 0);
  iupImageStockSet("IUP_IconMessageHelp", load_image_dialog_question, 0);
  iupImageStockSet("IUP_IconMessageInfo", load_image_dialog_information, 0);
  iupImageStockSet("IUP_IconMessageSecurity", load_image_dialog_password, 0);
  iupImageStockSet("IUP_IconMessageWarning", load_image_dialog_warning, 0);
}
