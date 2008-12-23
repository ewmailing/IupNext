-- IupTree Example in IupLua 
-- Creates a tree with some branches and leaves. 
-- Two callbacks are registered: one deletes marked nodes when the Del key 
-- is pressed, and the other, called when the right mouse button is pressed, 
-- opens a menu with options. 

require( "iuplua" )
require( "iupluacontrols" )

tree = iup.tree{}

function tree:showrename_cb(id)
  print("SHOWRENAME")
  return iup.DEFAULT
end
-- Callback called when a node will be renamed
function tree:renamenode_cb(id)
  print("RENAMENODE")
  return iup.DEFAULT
end


function tree:k_any(c)
  if c == 316 then tree.delnode = "MARKED" end
  return iup.DEFAULT
end


function init_tree_atributes()
  tree.font = "COURIER_NORMAL_10"
  tree.name = "Figures"
  tree.addbranch = "3D"
  tree.addbranch = "2D"
  tree.addbranch1 = "parallelogram"
  tree.addleaf2 = "diamond"
  tree.addleaf2 = "square"
  tree.addbranch1 = "triangle"
  tree.addleaf2 = "scalenus"
  tree.addleaf2 = "isoceles"
  tree.value = "6"
  tree.ctrl = "YES"
  tree.shift = "YES"
  tree.addexpanded = "NO"
  tree.redraw = "YES"
  tree.showrename = "NO"
end


dlg = iup.dialog{tree; title = "IupTree", size = "QUARTERxTHIRD"} 
dlg:showxy(iup.CENTER,iup.CENTER)
init_tree_atributes()

if (not iup.MainLoopLevel or iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
