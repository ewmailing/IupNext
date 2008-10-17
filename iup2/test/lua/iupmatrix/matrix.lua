
mat = iupmatrix {
         numlin = 3,
         numcol = 1,
         numlin_visible = 3,
         numcol_visible = 1,
         resizematrix = "YES",
      }

data = {{"AAA"},{"BBB"},{"CCC"}}

function mat:valuecb(lin, col)
  if lin == 0 and col == 0 then
    return "TITLE 0:0" 
  elseif lin == 0 and col == 1 then
    return "TITLE 0:1"
  end
  return data[lin][col]
end

function value_edit(lin, col, newval)
  data[lin][col] = newval
  return IUP_DEFAULT
end

function mat:click(lin, col, r) 
  print("click:", self, lin, col, r)
end

dlg = iupdialog{mat; title="Testing Matrix Control"}
dlg:popup( IUP_CENTER, IUP_CENTER )
