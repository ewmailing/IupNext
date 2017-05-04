------------------------------------------------------------------------------
-- ScintillaDlg class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "scintilladlg",
  parent = iup.WIDGET,
  subdir = "ctrl",
  creation = "",
  callback = {
    markerchanged_cb = "nnn",
  },
  include = "iup_scintilla.h",
  funcname = "ScintillaDlg",
} 

function ctrl.showxy(ih, x, y)
  return iup.ShowXY(ih, x, y)
end

function ctrl.popup(ih, x, y)
  iup.Popup(ih,x,y)
end

function ctrl.createElement(class, param)
   return iup.ScintillaDlg()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
