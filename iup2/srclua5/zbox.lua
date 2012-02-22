------------------------------------------------------------------------------
-- ZBox class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "zbox",
  parent = iup.BOX,
  creation = "-",
  callback = {}
}

function ctrl.append (handle, elem)
  iup.SetNameHandle(elem)
  iup.Append(handle, elem)
end

function ctrl.SetChildrenNames(obj)
  if obj then
    local i = 1
    while obj[i] do
      iup.SetNameHandle(obj[i])
      i = i+1
    end
  end
end

function ctrl.createElement(class, arg)
   ctrl.SetChildrenNames(arg)
   return iup.Zbox()
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
