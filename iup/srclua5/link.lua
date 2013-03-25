------------------------------------------------------------------------------
-- Link class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "link",
  parent = iup.WIDGET,
  creation = "SS",
  callback = {
    action = "s", 
  }
}

function ctrl.createElement(class, param)
   return iup.Link(param[1], param[2])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
