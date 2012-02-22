------------------------------------------------------------------------------
-- Text class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "text",
  parent = iup.WIDGET,
  creation = "-", 
  callback = {
    action = "ns",
    caret_cb = "nn", 
  }
}

function ctrl.createElement(class, arg)
   return iup.Text()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
