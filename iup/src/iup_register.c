/** \file
* \brief Register the Internal Controls
*
* See Copyright Notice in "iup.h"
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "iup.h"

#include "iup_object.h"
#include "iup_register.h"
#include "iup_stdcontrols.h"


static Itable *iregister_table = NULL;   /* table indexed by name containing Iclass* address */

void iupRegisterInit(void)
{
  iregister_table = iupTableCreate(IUPTABLE_STRINGINDEXED);
}

void iupRegisterFinish(void)
{
  char* name = iupTableFirst(iregister_table);
  while (name)
  {
    Iclass* ic = (Iclass*)iupTableGetCurr(iregister_table);
    iupClassRelease(ic);
    name = iupTableNext(iregister_table);
  }

  iupTableDestroy(iregister_table);
  iregister_table = NULL;
}

Iclass* iupRegisterFindClass(const char* name)
{
  return (Iclass*)iupTableGet(iregister_table, name);
}

void iupRegisterClass(Iclass* ic)
{
  iupTableSet(iregister_table, ic->name, (void*)ic, IUPTABLE_POINTER);
}


/***************************************************************/

void iupRegisterInternalClasses(void)
{
  iupRegisterClass(iupDialogGetClass());
  iupRegisterClass(iupMessageDlgGetClass());
  iupRegisterClass(iupColorDlgGetClass());
  iupRegisterClass(iupFontDlgGetClass());
  iupRegisterClass(iupFileDlgGetClass());

  iupRegisterClass(iupTimerGetClass());
  iupRegisterClass(iupImageGetClass());
  iupRegisterClass(iupImageRGBGetClass());
  iupRegisterClass(iupImageRGBAGetClass());
  iupRegisterClass(iupUserGetClass());

  iupRegisterClass(iupRadioGetClass());
  iupRegisterClass(iupFillGetClass());
  iupRegisterClass(iupHboxGetClass());
  iupRegisterClass(iupVboxGetClass());
  iupRegisterClass(iupZboxGetClass());
  iupRegisterClass(iupCboxGetClass());
  iupRegisterClass(iupNormalizerGetClass());

  iupRegisterClass(iupMenuGetClass());
  iupRegisterClass(iupItemGetClass());
  iupRegisterClass(iupSeparatorGetClass());
  iupRegisterClass(iupSubmenuGetClass());

  iupRegisterClass(iupLabelGetClass());
  iupRegisterClass(iupButtonGetClass());
  iupRegisterClass(iupToggleGetClass());
  iupRegisterClass(iupCanvasGetClass());
  iupRegisterClass(iupFrameGetClass());
  iupRegisterClass(iupTextGetClass());
  iupRegisterClass(iupMultilineGetClass());
  iupRegisterClass(iupListGetClass());

  iupRegisterClass(iupProgressBarGetClass());
  iupRegisterClass(iupValGetClass());
  iupRegisterClass(iupTabsGetClass());
  iupRegisterClass(iupSpinGetClass());
  iupRegisterClass(iupSpinboxGetClass());
}
