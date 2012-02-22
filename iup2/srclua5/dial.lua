------------------------------------------------------------------------------
-- Dial class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "dial",
  parent = iup.WIDGET,
  creation = "s",
  callback = {
     mousemove_cb = "d",       -- already registered by the val, but has a name conflict
--     button_press_cb = "d",    -- already registered by the val
--     button_release_cb = "d",  -- already registered by the val
  },
  include = "iupdial.h",
}

function ctrl.createElement(class, arg)
   return iup.Dial(arg[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
