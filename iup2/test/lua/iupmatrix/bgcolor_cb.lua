if not math then
  math = {}
  math.mod = mod
end

matrix = iup.matrix
{
    numlin=3,
    numcol=3,
    numcol_visible=3,
    height0=10,
    widthdef=30,
    scrollbar="VERTICAL",
}


--function matrix:value_cb(l, c) 
--  if l == 0 or c == 0
--    then return "fixed"
--    else return "value"
--  end
--end


--function matrix:value_edit_cb()
--  return iup.IGNORE
--end


function matrix:bgcolor_cb(l, c)
  if l == 0 or c == 0 then
    return iup.IGNORE
  else
    if math.mod(l,2) == 0
      then return 255, 0, 0, iup.DEFAULT
      else return 0, 255, 0, iup.DEFAULT
    end
  end
end


dlg=iup.dialog{matrix; title="IupMatrix in Callback Mode" }
dlg:show()
