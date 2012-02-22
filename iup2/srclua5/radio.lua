------------------------------------------------------------------------------
-- Radio class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "radio",
  parent = iup.WIDGET,
  creation = "i",
  callback = {}
} 

function ctrl.CreateChildrenNames(obj)
  if obj then
    if obj.parent.parent == BOX then
      local i = 1
      while obj[i] do
        ctrl.CreateChildrenNames (obj[i])
        i = i+1
      end
    elseif obj.parent == IUPFRAME then
      ctrl.CreateChildrenNames (obj[1])
    else
      iup.SetNameHandle(obj)
    end
  end
end

function ctrl.createElement(class, arg)
   ctrl.CreateChildrenNames(arg[1])
   return iup.Radio(arg[1])
end
   
iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
