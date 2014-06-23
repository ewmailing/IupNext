------------------------------------------------------------------------------
-- GLFrame class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "glframe",
  funcname = "GLFrame",
  include = "iupglcontrols.h",
  parent = iup.WIDGET,
  subdir = "gl",
  creation = "I",
  callback = {
  }
}

function ctrl.createElement(class, param)
   return iup.GLFrame(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
