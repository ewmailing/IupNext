
cv = iup.canvas{expand="YES"}

mat = iup.matrix {numcol=2, numlin=3,numcol_visible=2, 
                  numlin_visible=3, widthdef=34, expand="NO"}
mat:setcell(0,0,"Inflation") mat:setcell(1,0,"Medicine")
mat:setcell(2,0,"Food") mat:setcell(3,0,"Energy")
mat:setcell(0,1,"January 2000") mat:setcell(0,2,"February 2000")
mat:setcell(1,1,"5.6") mat:setcell(2,1,"2.2")
mat:setcell(3,1,"7.2") mat:setcell(1,2,"4.6")
mat:setcell(2,2,"1.3") mat:setcell(3,2,"1.4")

tg = iup.toggle{title="visible", value="ON", action=function(self, v)
  if v == 1 then
    mat.size = msize
    mat.visible = "YES"
--    dlg.size = size 
iup.Flush()   
print(mat.visible)
  else
    mat.visible = "NO"
    mat.size = "1x1"
--    dlg.size = nil
iup.Flush()   
print(mat.visible)
  end
end}

dlg = iup.dialog{iup.vbox{tg, iup.hbox{cv, mat}}; size="300x300"}
dlg:showxy(iup.CENTER,iup.CENTER)
size = dlg.size
msize = mat.size
