------------------------------------------------------------------------------
-- Tree class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "tree",
  parent = WIDGET,
  creation = "",
  callback = {
    selection_cb = "nn",
    multiselection_cb = "nn",  -- fake definition to be replaced by treefuncs module
    branchopen_cb = "n",
    branchclose_cb = "n",
    executeleaf_cb = "n",
    renamenode_cb = "ns",
    rename_cb = "ns",
    showrename_cb = "n",
    rightclick_cb = "n",
    dragdrop_cb = "nnnn",
  },
  include = "iupcontrols.h",
  extrafuncs = 1,
}

function TreeSetValueRec(handle, t, id)
  if t == nil then return end
  local cont = table.getn(t)
  while cont >= 0 do
    if type (t[cont]) == "table" then
      if t[cont].branchname ~= nil then
        SetAttribute(handle, "ADDBRANCH"..id, t[cont].branchname)
        TreeSetValueRec(handle, t[cont], id+1)
      end
    else
      if t[cont] then
        SetAttribute(handle, "ADDLEAF"..id, t[cont])
      end
    end
    cont = cont - 1
   end
end

function TreeSetValue(handle, t)
  if t.branchname ~= nil then
    SetAttribute(handle, "NAME", t.branchname)
  end
  TreeSetValueRec(handle, t, 0)
end

function ctrl.createElement(class, arg)
  return Tree()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
