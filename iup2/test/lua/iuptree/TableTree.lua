
function TableTreeRec(tree, t, largura)

  if t == nil then return end

  local index, value = next(t, nil)

  while index do
    if type (t[index]) == "table" then  
      IupSetAttribute(tree, "ADDBRANCH"..largura, "("..index..")")
      TableTreeRec(tree, value, largura+1)
    else
      IupSetAttribute(tree, "ADDLEAF"..largura, value .." ("..index..")")
    end
    index, value = next(t, index)
   end 
end

function TableTree(t)
  tree = iuptree{}
  dlg  = iupdialog{tree ; title = "TableTree result", size = "200x200"}
  dlg:showxy(IUP_CENTER,IUP_CENTER)

  IupSetAttribute(tree, "NAME", "Results from given table")
  TableTreeRec(tree, t, 0)
  tree.redraw = "YES"
end
