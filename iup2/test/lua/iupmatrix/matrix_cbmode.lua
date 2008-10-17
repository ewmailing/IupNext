matrix = iup.matrix
{
    numlin=3,
    numcol=3,
    numcol_visible=3,
    height0=10,
    widthdef=30,
    scrollbar="VERTICAL",
    mark_mode="CELL",
    multiple="yes"
}

data = {
        {"1:1", "1:2", "1:3"}, 
        {"2:1", "2:2", "2:3"}, 
        {"3:1", "3:2", "3:3"}, 
       }

function matrix:bgcolor_cb(l, c)
  if l == 0 or c == 0 then
    return iup.IGNORE
  else
    if l == 2 then 
      return 255, 0, 0, iup.DEFAULT -- red when line equals 2
    else 
      return 0, 0, 255, iup.DEFAULT -- blue elsewhere
    end
  end
end

function matrix:value_cb(l, c) 
  if l == 0 or c == 0 then
    return "title"
  end
  return data[l][c]
end

function matrix:value_edit_cb(l, c, newvalue)
  data[l][c] = newvalue
end

dlg=iup.dialog{matrix; title="IupMatrix in Callback Mode" }
dlg:show()
