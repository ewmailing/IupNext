_matrix = iup.matrix
{
    numlin=3,
    numcol=3,
    numcol_visible=3,
    height0=10,
    widthdef=30,
    scrollbar="VERTICAL",
}   
    
_data = { 
        {"1:1", "1:2", "1:3"},
        {"2:1", "2:2", "2:3"}, 
        {"3:1", "3:2", "3:3"},
       }
  
function _matrix:value_cb(l, c)
print("[_value_cb] ("..l..", "..c..")")
  if l == 0 or c == 0 then
    return "title"
  end
  return _data[l][c]
end
  
function _matrix:value_edit_cb(l, c, newvalue) print("[_value_edit_cb] ("..l..", "..c..") = "..tostring(newvalue))
  _data[l][c] = newvalue
end
  
function _matrix:enteritem_cb(l, c)
print("[_enteritem_cb] ("..l..", "..c..")")
  return iup.DEFAULT
end 

dlg=iup.dialog{_matrix;title="teste"}
dlg:show()
