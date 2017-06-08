------------------------------------------------------------------------------
-- LuaScripterDlg class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "luascripterdlg",
  parent = iup.WIDGET,
  subdir = "scripter",
  creation = "",
  callback = {
  },
  include = "iupluascripterdlg.h",
  funcname = "LuaScripterDlg",
} 

function ctrl.showxy(ih, x, y)
  return iup.ShowXY(ih, x, y)
end

function ctrl.popup(ih, x, y)
  iup.Popup(ih,x,y)
end

function ctrl.createElement(class, param)
   return iup.LuaScripterDlg()
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
