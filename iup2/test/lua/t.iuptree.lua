local tree1 = iup.tree{
  rastersize = "300x100", 
}

local tree2 = iup.tree{
  rastersize = "100x100", 
}

local canvas = iup.canvas{
  rastersize = "200x200", 
}

local function UpdateTrees()
   tree1.redraw = "YES"
   tree2.redraw = "YES"
   canvas.redraw = "YES"
   iup.SetIdle(nil)
end

local function PostUpdateTrees()
   iup.SetIdle(UpdateTrees)   
end

function InitTree(tree)
  tree.name = "Curvas"
  tree.addbranch = "Ajuste"
  tree.addbranch = "Correlacao"
  tree.addbranch = "Plug"
  tree.addbranch3 = "Krow"
  tree.addbranch3 = "Krgw"
  tree.addbranch3 = "Krog"
  tree.addbranch3 = "Pcgo"
  tree.addbranch2 = "Krow"
  tree.addbranch2 = "Krgw"
  tree.addbranch2 = "Krog"
  tree.addbranch2 = "Pcgo"
  tree.addbranch1 = "Krow"
  tree.addbranch1 = "Krgw"
  tree.addbranch1 = "Krog"
  tree.addbranch1 = "Pcgo"
  for i=1,18 do
    tree.addleaf2 = "leaf"
  end
  
  tree.selection_cb = function(self,id,s)
     PostUpdateTrees()
  end

  tree.value = "0"
  tree.addexpanded = "NO"
  tree.redraw = "YES"
end

InitTree(tree1)
InitTree(tree2)


local dlg = iup.dialog{
   iup.hbox{
      iup.vbox{
         tree1,
         tree2,
      },
      canvas,
   }
   ; title = "Tree Test", rastersize = "500x500"
}

dlg:showxy(iup.CENTER, iup.CENTER)