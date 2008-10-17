/** \file
 * \brief iupspin control
 *
 * See Copyright Notice in iup.h
 * $Id: ispin.c,v 1.1 2008-10-17 06:19:20 scuri Exp $
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iglobal.h"
#include "idrv.h"
#include "iupkey.h"
#include "icpi.h"
#include "iupspin.h"

static Ihandle* spin_timer = NULL;

static int callSpinCB(Ihandle* self, int dub, int ten, int sign)
{
  Ihandle* box = (Ihandle*)IupGetAttribute(self, "_IUPSPIN_BOX");
  IFni cb = (IFni) IupGetCallback(box, "SPIN_CB");
  if (cb) 
  {
    return cb(box, sign*(dub && ten? 100:
                        ten? 10: 
                        dub? 2: 1));
  }
  return IUP_DEFAULT;
}

static int iupspin_timer_cb(Ihandle *self)
{
  Ihandle* spin_self = (Ihandle*)IupGetAttribute(self, "_IUPSPIN_SELF");
  char* status = IupGetAttribute(self, "_IUPSPIN_STATUS");
  int spin_dir = IupGetInt(self, "_IUPSPIN_DIR");
  int count = IupGetInt(self, "_IUPSPIN_COUNT");
  char* reconfig = NULL;

  if (count == 0) /* first time */
    reconfig = "50";
  else if (count == 14) /* 300 + 14*50 = 1000 (1 second) */
    reconfig = "25";
  else if (count == 34) /* 300 + 14*50 + 20*50 = 2000 (2 seconds) */
    reconfig = "10";

  if (reconfig)
  {
    IupSetAttribute(spin_timer, "RUN", "NO");
    IupSetAttribute(spin_timer, "TIME", reconfig);
    IupSetAttribute(spin_timer, "RUN", "YES");
  }
  IupSetfAttribute(spin_timer, "_IUPSPIN_COUNT", "%d", count+1);
  return callSpinCB(spin_self, isshift(status), iscontrol(status), spin_dir);
}

static void iupspinPrepareTimer(Ihandle *self, char* status, char* dir)
{
  IupSetAttribute(spin_timer, "_IUPSPIN_SELF", (char*)self);
  IupStoreAttribute(spin_timer, "_IUPSPIN_STATUS", status);
  IupSetAttribute(spin_timer, "_IUPSPIN_DIR", dir);
  IupSetAttribute(spin_timer, "_IUPSPIN_COUNT", "0");
  IupSetAttribute(spin_timer, "TIME", "400");
  IupSetAttribute(spin_timer, "RUN", "YES");
}

static int iupspin_k_sp(Ihandle *self)
{
  int dir = IupGetInt(self, "_IUPSPIN_DIR");
  return callSpinCB(self, 0, 0, dir);
}

static int iupspin_k_ssp(Ihandle *self)
{
  int dir = IupGetInt(self, "_IUPSPIN_DIR");
  return callSpinCB(self, 1, 0, dir);
}

static int iupspin_k_csp(Ihandle *self)
{
  int dir = IupGetInt(self, "_IUPSPIN_DIR");
  return callSpinCB(self, 0, 1, dir);
}

static int iupspin_button_cb(Ihandle *self, int but, int pressed, int x, int y, char* status)
{
  (void)x;
  (void)y;
  if (pressed && but == IUP_BUTTON1)
  {
    int dir = IupGetInt(self, "_IUPSPIN_DIR");
    iupspinPrepareTimer(self, status, IupGetAttribute(self, "_IUPSPIN_DIR"));
    return callSpinCB(self, isshift(status), iscontrol(status), dir);
  }
  else if (!pressed && but == IUP_BUTTON1)
  {
    IupSetAttribute(spin_timer, "RUN", "NO");
  }
  return IUP_DEFAULT;
}

Ihandle* IupSpin(void)
{
  Ihandle *spin, *bt_up, *bt_down;

  bt_up = IupButton("", NULL);
  IupSetAttribute(bt_up, IUP_EXPAND, "NO");
  IupSetAttribute(bt_up, IUP_IMAGE, "IupSpinUpImage");
  IupSetAttribute(bt_up, "_IUPSPIN_DIR", "1");
  IupSetCallback(bt_up, IUP_BUTTON_CB, (Icallback) iupspin_button_cb);
  IupSetCallback(bt_up, "K_SP", (Icallback) iupspin_k_sp);
  IupSetCallback(bt_up, "K_sSP", (Icallback) iupspin_k_ssp);
  IupSetCallback(bt_up, "K_cSP", (Icallback) iupspin_k_csp);

  bt_down = IupButton("", NULL);
  IupSetAttribute(bt_down, IUP_EXPAND, "NO");
  IupSetAttribute(bt_down, IUP_IMAGE, "IupSpinDownImage");
  IupSetAttribute(bt_down, "_IUPSPIN_DIR", "-1");
  IupSetCallback(bt_down, IUP_BUTTON_CB, (Icallback) iupspin_button_cb);
  IupSetCallback(bt_down, "K_SP", (Icallback) iupspin_k_sp);
  IupSetCallback(bt_down, "K_sSP", (Icallback) iupspin_k_ssp);
  IupSetCallback(bt_down, "K_cSP", (Icallback) iupspin_k_csp);

  spin = IupVbox(bt_up, bt_down, NULL);
  IupSetAttribute(spin, IUP_GAP, "0");
  IupSetAttribute(spin, IUP_MARGIN, "0x0");
  IupSetAttribute(spin, "_IUPSPIN_BOX", (char*)spin);
 
  return spin;
}

Ihandle* IupSpinbox(Ihandle* ctrl)
{
  Ihandle* spin = IupSpin();
  Ihandle* box;

  box = IupHbox(ctrl, spin, NULL);
  IupSetAttribute(box, IUP_GAP, "0");
  IupSetAttribute(box, IUP_MARGIN, "0x0");
  IupSetAttribute(box, "ALIGNMENT", "ACENTER");
  IupSetAttribute(spin, "_IUPSPIN_BOX", (char*)box);

  return box;
}

static unsigned char iupspin_up_img[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0};

static unsigned char iupspin_down_img[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1};

void IupSpinOpen(void)
{
  Ihandle* img;

  img = IupImage(9,6, iupspin_up_img);
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "BGCOLOR"); 
  IupSetHandle ("IupSpinUpImage", img); 

  img = IupImage(9,6, iupspin_down_img);
  IupSetAttribute (img, "0", "0 0 0"); 
  IupSetAttribute (img, "1", "BGCOLOR"); 
  IupSetHandle ("IupSpinDownImage", img); 

  spin_timer = IupTimer();
  IupSetCallback(spin_timer, "ACTION_CB", (Icallback) iupspin_timer_cb);
}

void IupSpinClose(void)
{
  IupDestroy(IupGetHandle("IupSpinUpImage"));
  IupDestroy(IupGetHandle("IupSpinDownImage"));
  IupDestroy(spin_timer);
}
