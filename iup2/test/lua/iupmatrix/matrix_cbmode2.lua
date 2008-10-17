matrix = iup.matrix
{
    numlin=3,
    numcol=3,
    numcol_visible=3,
    height0=10,
    widthdef=30,
		--vvv
		mark_mode="CELL", 
		multiple="YES",
		area="NOT_CONTINUOUS",
		--^^^
    scrollbar="VERTICAL",
}

data = {
        {"1:1", "1:2", "1:3"}, 
        {"2:1", "2:2", "2:3"}, 
        {"3:1", "3:2", "3:3"}, 
       }

function matrix:value_cb(l, c) 
  if l == 0 or c == 0 then
    return "title"
  end
  return data[l][c]
end

function matrix:value_edit_cb(l, c, newvalue)
  data[l][c] = newvalue
end

--[[
function matrix:markedit_cb(lin,col,marked)
	if lin==0 then print(marked) end
end
--]]

dlg=iup.dialog{matrix; title="IupMatrix in Callback Mode" }
dlg:show()

iup.MainLoop()