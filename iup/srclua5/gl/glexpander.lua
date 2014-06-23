------------------------------------------------------------------------------
-- GLExpander class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "glexpander",
  parent = iup.WIDGET,
  funcname = "GLExpander",
  include = "iupglcontrols.h",
  creation = "I",
  subdir = "gl",
  callback = {
    action = "", 
  }
}

function ctrl.createElement(class, param)
  return iup.GLExpander(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
