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
    noderemoved_cb = "s",  -- fake definition to be replaced by treefuncs module
    rename_cb = "ns",
    showrename_cb = "n",
    rightclick_cb = "n",
    dragdrop_cb = "nnnn",
  },
  extrafuncs = 1,
}

function TreeSetNodeAttributes(handle, id, attrs)
  for attr, val in pairs(attrs) do
    handle[attr..id] = val
  end
end

function TreeSetAncestorsAttributes(handle, ini, attrs)
  ini = handle["parent"..ini]
  local stack = {}
    while ini do
      table.insert(stack, 1, ini)
      ini = handle["parent"..ini]
    end
  for i = 1, #stack do
    TreeSetNodeAttributes(handle, stack[i], attrs)
  end
end

function TreeSetDescentsAttributes(handle, ini, attrs)
  local id = ini
  for i = 1, handle["childcount"..ini] do
    id = id+1
    TreeSetNodeAttributes(handle, id, attrs)
    if handle["kind"..id] == "BRANCH" then
      id = TreeSetDescentsAttributes(handle, id, attrs)
    end
  end
  return id
end

local function TreeSetAttributeHandle(handle, name, value)
   if iupGetClass(value) == "iup handle" then value = ihandle_setname(value) end
   SetAttribute(handle, name, value)
end

-- must be after the branch has nodes
function TreeSetState(handle, tnode, id)
  if tnode.state then SetAttribute(handle, "STATE"..id, tnode.state) end
end

function TreeSetNodeAttrib(handle, tnode, id)
  if tnode.color then SetAttribute(handle, "COLOR"..id, tnode.color) end
  if tnode.titlefont then SetAttribute(handle, "TITLEFONT"..id, tnode.titlefont) end
  if tnode.marked then SetAttribute(handle, "MARKED"..id, tnode.marked) end
  if tnode.image then TreeSetAttributeHandle(handle, "IMAGE"..id, tnode.image) end
  if tnode.imageexpanded then TreeSetAttributeHandle(handle, "IMAGEEXPANDED"..id, tnode.imageexpanded) end
  if tnode.userid then TreeSetUserId(handle, id, tnode.userid) end
end

function TreeAddNodesRec(handle, t, id)
  if t == nil then return end
  local cont = #t
  while cont >= 0 do
    local tnode = t[cont]
    if type(tnode) == "table" then
      if tnode.branchname then
        SetAttribute(handle, "ADDBRANCH"..id, tnode.branchname)
        TreeSetNodeAttrib(handle, tnode, id+1)
        TreeAddNodesRec(handle, tnode, id+1)
        TreeSetState(handle, tnode, id+1)
      elseif tnode.leafname then
        SetAttribute(handle, "ADDLEAF"..id, tnode.leafname)
        TreeSetNodeAttrib(handle, tnode, id+1)
      end
    else
      if tnode then
        SetAttribute(handle, "ADDLEAF"..id, tnode)
      end
    end
    cont = cont - 1
   end
end

function TreeAddNodes(handle, t, id)
  if (not id) then
    id = 0  -- default is the root
    if t.branchname then SetAttribute(handle, "TITLE0", t.branchname) end
    TreeSetNodeAttrib(handle, t, 0)
  end
  TreeAddNodesRec(handle, t, id)
  if (id == 0) then TreeSetState(handle, t, 0) end
end

-- backward compatibility
TreeSetValue = TreeAddNodes

function ctrl.createElement(class, param)
  return Tree()
end

iupRegisterWidget(ctrl)
iupSetClass(ctrl, "iup widget")
