-- Creates iup.tree
tree = iup.tree {}

-- Creates rename dialog
ok = iup.button{title = "OK",size="EIGHTH"}
cancel = iup.button{title = "Cancel",size="EIGHTH"}
text = iup.text{border="YES",expand="YES"}
dlg_rename = iup.dialog{iup.vbox{text,iup.hbox{ok,cancel}}
 ; defaultenter=ok,
   defaultesc=cancel,
   title="Enter node's name",
   size="QUARTER",
   startfocus=text}

-- Creates menu displayed when the right mouse button is pressed
  addleaf = iup.item {title = "Add Leaf"}
  addbranch = iup.item {title = "Add Branch"}
  renamenode = iup.item {title = "Rename Node"}
  menu = iup.menu{addleaf, addbranch, renamenode}

-- Callback of the right mouse button click
function tree:rightclick(id)
  tree.value = id
  tree.redraw = "YES"
  menu:popup(iup.MOUSEPOS,iup.MOUSEPOS)

  return iup.DEFAULT
end

-- Callback called when a node will be renamed
function tree:renamenode(id)
  text.value = tree.name

  dlg_rename:popup(iup.CENTER, iup.CENTER)
  iup.SetFocus(tree)
  
  return iup.DEFAULT
end

-- Callback called when the rename operation is cancelled
function cancel:action()
  return iup.CLOSE
end

-- Callback called when the rename operation is confirmed
function ok:action()
  tree.name = text.value

  return iup.CLOSE
end

function tree:k_any(c)
  if c == 339 then tree.delnode = "MARKED" end
  return iup.DEFAULT
end

-- Callback called when a leaf is added
function addleaf:action()
  iup.SetAttribute(tree,"ADDLEAF","")
  tree.redraw = "YES"
  return iup.DEFAULT
end

-- Callback called when a branch is added
function addbranch:action()
  iup.SetAttribute(tree,"ADDBRANCH","")
  tree.redraw = "YES"
  return iup.DEFAULT
end

-- Callback called when a branch will be renamed
function renamenode:action()
  tree:renamenode(tree.value)
  tree.redraw = "YES"
  return iup.DEFAULT
end

function init_tree_atributes()
  iup.SetAttribute(tree, "FONT","COURIER_NORMAL_10")
  iup.SetAttribute(tree, "NAME","Figures")
  iup.SetAttribute(tree, "ADDBRANCH","3D")
  iup.SetAttribute(tree, "ADDBRANCH","2D")
  iup.SetAttribute(tree, "ADDBRANCH1","parallelogram")
  iup.SetAttribute(tree, "ADDLEAF2","diamond")
  iup.SetAttribute(tree, "ADDLEAF2","square")
  iup.SetAttribute(tree, "ADDBRANCH1","triangle")
  iup.SetAttribute(tree, "ADDLEAF2","scalenus")
  iup.SetAttribute(tree, "ADDLEAF2","isoceles")
  iup.SetAttribute(tree, "ADDLEAF2","equilateral")
  iup.SetAttribute(tree, "VALUE","6")
  iup.SetAttribute(tree, "CTRL","ON")
  iup.SetAttribute(tree, "SHIFT","ON")
  iup.SetAttribute(tree, "ADDEXPANDED", "ON")
  tree.redraw = "YES"
end

dlg = iup.dialog{tree ; title = "IupTree", size = "QUARTERxTHIRD"} 
dlg:showxy(iup.CENTER,iup.CENTER)
init_tree_atributes()   

--IupTreeSetTableId = IupTreeSetUserId
--IupTreeGetTable   = IupTreeGetUserId
--IupTreeGetTableId = IupTreeGetId

-- Possibilidade 1 (guardando tabelas):
print("TEST1--------------------------")
t={name="vazio"}
iup.TreeSetTableId(tree, 1, t)
--iup.SetAttribute(tree,"ADDLEAF0","INSERIDO")
--tree.redraw = "YES"
print(1, iup.TreeGetTableId(tree, t))
b = iup.TreeGetTable(tree, 1)
print(t, b)
assert(b.name == "vazio")
assert(type(b) == type(t))

-- Possibilidade 2 (guardando userdata):
print("\nTEST2--------------------------")
t={name="ta'vazio"}
iup.TreeSetUserId(tree, 2, tree)
print(2, iup.TreeGetId(tree, tree))
b = iup.TreeGetUserId(tree, 2)
print(tree, b)
if tag then
  print(tag(tree), tag(b))
end  

print("\nTEST3--------------------------")
iup.TreeSetTableId(tree, 1, nil)
print("nil", iup.TreeGetTableId(tree, t))
print("nil", iup.TreeGetTable(tree, 1))

print("\nTEST4--------------------------")
iup.TreeSetUserId(tree, 2, nil)
print("nil", iup.TreeGetId(tree, tree))
print("nil", iup.TreeGetUserId(tree, 2))

print("Teste completou")
