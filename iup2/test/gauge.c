
#include <stdio.h>

#include "iup.h"
#include "iupcontrols.h"

double speed = 0.00001;
Ihandle *gauge = NULL;

static char pixmap_play[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static char pixmap_start[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,1,1,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static char pixmap_rewind[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static char pixmap_forward[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,1,2,2,1,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,1,1,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

static char pixmap_show[] = 
{ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2
 ,2,2,2,2,2,1,2,2,2,1,1,1,2,2,2,2,1,2,2,2,2,2
 ,2,2,2,2,1,2,2,2,1,1,2,2,1,2,2,2,2,1,2,2,2,2
 ,2,2,2,1,2,2,2,2,1,1,1,2,1,2,2,2,2,2,1,2,2,2
 ,2,2,2,2,1,2,2,2,1,1,1,1,1,2,2,2,2,1,2,2,2,2
 ,2,2,2,2,2,1,2,2,2,1,1,1,2,2,2,2,1,2,2,2,2,2
 ,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 ,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };

void createimg_s (void)
{ 
  Ihandle *img_start, *img_play, *img_forward, *img_rewind, *img_show;
  
  img_start = IupImage(22,22, pixmap_start);
  img_play = IupImage(22,22, pixmap_play);
  img_forward = IupImage(22,22, pixmap_forward);
  img_rewind = IupImage(22,22, pixmap_rewind);
  img_show = IupImage(22,22, pixmap_show);
  
  IupSetHandle ("img_start", img_start); 
  IupSetHandle ("img_play", img_play);
  IupSetHandle ("img_forward", img_forward); 
  IupSetHandle ("img_rewind", img_rewind);
  IupSetHandle ("img_show", img_show);
  
  IupSetAttribute (img_start, "1", "0 0 0"); 
  IupSetAttribute (img_start, "2", IUP_BGCOLOR);
  IupSetAttribute (img_play, "1", "0 0 0"); 
  IupSetAttribute (img_play, "2", IUP_BGCOLOR); 
  IupSetAttribute (img_forward, "1", "0 0 0"); 
  IupSetAttribute (img_forward, "2", IUP_BGCOLOR); 
  IupSetAttribute (img_rewind, "1", "0 0 0"); 
  IupSetAttribute (img_rewind, "2", IUP_BGCOLOR); 
  IupSetAttribute (img_show, "1", "0 0 0"); 
  IupSetAttribute (img_show, "2", IUP_BGCOLOR); 
}

int idle_cb(void)
{
  char newvalue[40];
  double value = (double)IupGetFloat(gauge, IUP_VALUE);
  
  value += speed;
  
  if(value > IupGetFloat(gauge, IUP_MAX)) value = IupGetFloat(gauge, IUP_MIN);
  
  sprintf(newvalue, "%.7f",value);
  
  IupSetAttribute(gauge, IUP_VALUE, newvalue);

  return IUP_DEFAULT;
}

int btn_pause_cb(void)
{
  if(!IupGetFunction(IUP_IDLE_ACTION))
    IupSetFunction(IUP_IDLE_ACTION, (Icallback) idle_cb);
  else
    IupSetFunction(IUP_IDLE_ACTION, NULL);
  
  return IUP_DEFAULT;
}

int btn_start_cb(void)
{
  IupSetAttribute(gauge, IUP_VALUE, IupGetAttribute(gauge, IUP_MIN));
  return IUP_DEFAULT;
}

int btn_accelerate_cb(void)
{
  speed *= 2;
  if(speed > 1.0) speed = 1.0;
  return IUP_DEFAULT;
}

int btn_decelerate_cb(void)
{
  speed /= 2;
  return IUP_DEFAULT;
}

int btn_show_cb(void)
{
  if(!IupGetInt(gauge,ICTL_SHOW_TEXT))
  {
    IupSetAttribute (gauge, ICTL_SHOW_TEXT, IUP_YES);
    IupSetAttribute (gauge, ICTL_DASHED, IUP_NO);
  }
  else
  {
    IupSetAttribute (gauge, ICTL_SHOW_TEXT, IUP_NO);
    IupSetAttribute (gauge, ICTL_DASHED, IUP_YES);
  }
  
  return IUP_DEFAULT;
}

int main(int argc, char **argv)
{
  Ihandle *dlg;
  Ihandle *vbox;
  Ihandle *hbox;
  Ihandle *btn_start, *btn_pause, *btn_accelerate, *btn_decelerate, *btn_show;
  
  IupOpen(&argc, &argv);       
  IupControlsOpen();  
  
  gauge = IupGauge();
 
  IupSetAttribute(gauge,IUP_EXPAND,IUP_YES);
//  IupSetAttribute(gauge,IUP_MAX,"3.5");
//  IupSetAttribute(gauge,IUP_MIN,"0.7");

  btn_start = IupButton ( "start", "btn_start_act" );
  btn_pause = IupButton ( "pause", "btn_pause_act" );
  btn_accelerate = IupButton ( "accelerate", "btn_accelerate_act" );
  btn_decelerate = IupButton ( "decelerate", "btn_decelerate_act" );
  btn_show = IupButton ( "show", "btn_show_act" );

  createimg_s();

  IupSetAttributes( btn_start, "IMAGE = img_start, TIP = Start" );
  IupSetAttributes( btn_pause, "IMAGE = img_play, TIP = Pause" );
  IupSetAttributes( btn_accelerate, "IMAGE = img_forward, TIP = Accelerate" );
  IupSetAttributes( btn_decelerate, "IMAGE = img_rewind, TIP = Decelerate" );
  IupSetAttributes( btn_show, "IMAGE = img_show, TIP = Show" );

  hbox = IupHbox
         (
           IupFill(), 
           btn_pause,
           btn_start,
           btn_decelerate,
           btn_accelerate,
           btn_show,
           IupFill(),
           NULL
         );

  vbox = IupVbox ( gauge , hbox, NULL );
  IupSetAttribute(vbox, IUP_MARGIN, "10x10");
  IupSetAttribute(vbox, IUP_GAP, "5");

  dlg = IupDialog(vbox);
  
  IupSetAttributes(dlg, "TITLE=IupGauge");

  IupSetFunction( "btn_pause_act", (Icallback) btn_pause_cb );
  IupSetFunction( "btn_start_act", (Icallback) btn_start_cb );
  IupSetFunction( "btn_accelerate_act", (Icallback) btn_accelerate_cb );
  IupSetFunction( "btn_decelerate_act", (Icallback) btn_decelerate_cb );
  IupSetFunction( "btn_show_act", (Icallback) btn_show_cb );
  
  IupSetFunction( IUP_IDLE_ACTION, (Icallback) idle_cb);
  IupShowXY(dlg,IUP_CENTER,IUP_CENTER);
  IupMainLoop ();
  IupDestroy(dlg);

  IupDestroy(IupGetHandle ("img_start")); 
  IupDestroy(IupGetHandle ("img_play"));
  IupDestroy(IupGetHandle ("img_forward")); 
  IupDestroy(IupGetHandle ("img_rewind"));
  IupDestroy(IupGetHandle ("img_show"));

  IupControlsClose();  
  IupClose ();  
  return 0;
}

