--IupTree Example in IupLua
--Creates a tree with some branches and leaves. Uses a Lua Table to define the IupTree structure.

local function SelectionCb(self, id, status)
   if status == 1 then
      local node = IupTreeGetTable(self, id)
print("Selection():", self, id, node, IupTreeGetTableId(self, node))
   end
end

local tree1 = iuptree{
   ctrl = "YES",
   shift = "YES",
   selection = SelectionCb,
}
local tree2 = iuptree{
   ctrl = "YES",
   shift = "YES",
   selection = SelectionCb,
}


local function InitTrees(tr1, tr2)
   local root = {name = "raiz"}
   tr1["NAME0"] = root.name
   IupTreeSetTableId(tr1, 0, root)
   tr2["NAME0"] = root.name
   IupTreeSetTableId(tr2, 0, root)
   
   for i=4,1,-1 do
      local node = {name = "ramo"..i}
      tr1["ADDBRANCH0"] = node.name
      IupTreeSetTableId(tr1, 1, node)
      tr2["ADDBRANCH0"] = node.name
      IupTreeSetTableId(tr2, 1, node)
   
      for j=4,1,-1 do
         local leaf = {name = "folha"..j}
         tr1["ADDBRANCH1"] = leaf.name
         IupTreeSetTableId(tr1, 2, leaf)
         tr2["ADDBRANCH1"] = leaf.name
         IupTreeSetTableId(tr2, 2, leaf)
      end
   end
end 

InitTrees(tree1, tree2)
collectgarbage()   



local dlg = iupdialog{
   iuphbox{ tree1, tree2 }, 
   ; title = "TableTree result"
}
dlg:showxy(IUP_CENTER,IUP_CENTER)

tree1.redraw = "YES"
tree2.redraw = "YES"
IupMainLoop()


