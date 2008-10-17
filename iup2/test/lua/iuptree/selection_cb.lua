--IupTree Example in IupLua 
--Creates a tree with some branches and leaves. Two callbacks are registered: one deletes marked nodes when the Del key is pressed, and the other, called when the right mouse button is pressed, opens a menu with options. 

-- Creates iuptree
tree = iuptree {}

-- Creates rename dialog
ok = iupbutton{title = "OK",size="EIGHTH"}
cancel = iupbutton{title = "Cancel",size="EIGHTH"}
text = iuptext{border="YES",expand="YES"}
dlg_rename = iupdialog{iupvbox{text,iuphbox{ok,cancel}}
 ; defaultenter=ok,
   defaultesc=cancel,
   title="Enter node's name",
   size="QUARTER",
   startfocus=text}

-- Creates menu displayed when the right mouse button is pressed
  addleaf = iupitem {title = "Add Leaf"}
  addbranch = iupitem {title = "Add Branch"}
  renamenode = iupitem {title = "Rename Node"}
  menu = iupmenu{addleaf, addbranch, renamenode}

-- Callback of the right mouse button click
function tree:rightclick(id)
  tree.value = id
  menu:popup(IUP_MOUSEPOS,IUP_MOUSEPOS)

  return IUP_DEFAULT
end

-- Callback called when a node will be renamed
function tree:renamenode(id)
  text.value = tree.name

  dlg_rename:popup(IUP_CENTER, IUP_CENTER)
  IupSetFocus(tree)
  
  return IUP_DEFAULT
end

-- Callback called when the rename operation is cancelled
function cancel:action()
  return IUP_CLOSE
end

-- Callback called when the rename operation is confirmed
function ok:action()
  tree.name = text.value

  return IUP_CLOSE
end

function tree:k_any(c)
  if c == 339 then tree.delnode = "MARKED" end
  return IUP_DEFAULT
end

-- Callback called when a leaf is added
function addleaf:action()
  IupSetAttribute(tree,"ADDLEAF","")
  return IUP_DEFAULT
end

-- Callback called when a branch is added
function addbranch:action()
  IupSetAttribute(tree,"ADDBRANCH","")
  return IUP_DEFAULT
end

-- Callback called when a branch will be renamed
function renamenode:action()
  tree:renamenode(tree.value)
  return IUP_DEFAULT
end

function init_tree_atributes()
  IupSetAttribute(tree, "FONT",IUP_COURIER_NORMAL_10)
  IupSetAttribute(tree, "NAME","Figures")
  IupSetAttribute(tree, "ADDBRANCH","3D")
  IupSetAttribute(tree, "ADDBRANCH","2D")
  IupSetAttribute(tree, "ADDBRANCH1","parallelogram")
  IupSetAttribute(tree, "ADDLEAF2","diamond")
  IupSetAttribute(tree, "ADDLEAF2","square")
  IupSetAttribute(tree, "ADDBRANCH1","triangle")
  IupSetAttribute(tree, "ADDLEAF2","scalenus")
  IupSetAttribute(tree, "ADDLEAF2","isoceles")
  IupSetAttribute(tree, "ADDLEAF2","equilateral")
  IupSetAttribute(tree, "VALUE","6")
  IupSetAttribute(tree, "CTRL",IUP_YES)
  IupSetAttribute(tree, "SHIFT",IUP_YES)
  IupSetAttribute(tree, "ADDEXPANDED", IUP_NO)
  tree.redraw = "YES"
end

dlg = iupdialog{tree ; title = "IupTree", size = "QUARTERxTHIRD"} 
init_tree_atributes()

tree.rightclick = function() 
  IupMessage("Testing","RightClick callback ok") 
end

tree.selection = function() IupMessage("Testing","Selection callback ok")
end

dlg:showxy(IUP_CENTER,IUP_CENTER)
