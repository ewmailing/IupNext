------------------------------------------------------------------------------
-- ColorDlg class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "colordlg",
  parent = iup.WIDGET,
  subdir = "elem",
  creation = "",
  funcname = "ColorDlg",
  callback = {
    colorupdate_cb = "",
  }
} 

function ctrl.popup(handle, x, y)
  iup.Popup(handle,x,y)
end

function ctrl.createElement(class, param)
   return iup.ColorDlg()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")

