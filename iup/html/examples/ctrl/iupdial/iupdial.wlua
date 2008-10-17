--IupDial Example in IupLua 

require( "iuplua" )
require( "iupluacontrols" )

lbl_h = iup.label{title = "0", alignment = "ACENTER", size = "100x10"}
lbl_v = iup.label{title = "0", alignment = "ACENTER", size = "100x10"}
lbl_c = iup.label{title = "0", alignment = "ACENTER", size = "100x10"}

dial_v = iup.dial{"VERTICAL"; size="100x100"}
dial_h = iup.dial{"HORIZONTAL"; density=0.3}

function dial_v:mousemove_cb(a)
   lbl_v.title = a
   return iup.DEFAULT
end

function dial_v:button_press_cb(a)
   lbl_v.bgcolor = "255 0 0"
   return iup.DEFAULT
end

function dial_v:button_release_cb(a)
   lbl_v.bgcolor = nil
   return iup.DEFAULT
end

function dial_h:mousemove_cb(a)
   lbl_h.title = a
   return iup.DEFAULT
end

function dial_h:button_press_cb(a)
   lbl_h.bgcolor = "255 0 0"
   return iup.DEFAULT
end

function dial_h:button_release_cb(a)
   lbl_h.bgcolor = nil
   return iup.DEFAULT
end

dlg = iup.dialog
{
  iup.hbox
  {
    iup.fill{},
    iup.vbox
    {
      iup.fill{},
      iup.frame
      {
        iup.vbox
        {
          iup.hbox
          {
             iup.fill{},
             dial_v,
             iup.fill{}
          } ,
          iup.hbox
          {
             iup.fill{},
             lbl_v,
             iup.fill{}
          }
        }
      },
      iup.fill{},
      iup.frame
      {
        iup.vbox
        { 
          iup.hbox
          {
             iup.fill{},
             dial_h,
             iup.fill{}
          } ,
          iup.hbox
          {
             iup.fill{},
             lbl_h,
             iup.fill{}
          } ,
        } 
      },
      iup.fill{},
    },
    iup.fill{}
  }; title="IupDial"
}

dlg:showxy(iup.CENTER,iup.CENTER)

iup.MainLoop()
