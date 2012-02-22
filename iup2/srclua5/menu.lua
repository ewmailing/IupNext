------------------------------------------------------------------------------
-- Menu class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "menu",
  parent = iup.BOX,
  creation = "-",
  callback = {
    open_cb = "",
    menuclose_cb = "",
  }
}

function ctrl.popup(handle, x, y)
  iup.Popup(handle, x, y)
end

function ctrl.append(handle, elem)
  iup.Append(handle, elem)
end

function ctrl.createElement(class, arg)
  local n = #arg
  for i=1,n do
    if type(arg[i]) == "table" then 
      local itemarg = {}
      for u,v in pairs(arg[i]) do
        if type(u) ~= "number" then
          itemarg[u] = v
        end
      end
      if type(arg[i][1]) == "string" and (type(arg[i][2]) == "function" or type(arg[i][2]) == "string") then
        itemarg.title = arg[i][1]
        itemarg.action = arg[i][2]
        arg[i] = iup.item(itemarg)
      elseif type(arg[i][1]) == "string" and type(arg[i][2]) == "userdata" then
        itemarg[1] = arg[i][2]
        itemarg.title = arg[i][1]
        arg[i] = iup.submenu(itemarg)
      end
    end
  end
   return iup.Menu()
end

function ctrl.showxy(handle, x, y)
  return iup.ShowXY(handle, x, y)
end

function ctrl.destroy(handle)
  return iup.Destroy(handle)
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iup widget")
