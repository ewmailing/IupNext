------------------------------------------------------------------------------
-- GLScrollBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "glscrollbox",
  funcname = "GLScrollBox",
  include = "iupglcontrols.h",
  parent = iup.WIDGET,
  subdir = "gl",
  creation = "I",
  callback = {
  }
}

function ctrl.createElement(class, param)
   return iup.GLScrollBox(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
