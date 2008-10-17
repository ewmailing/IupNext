mat = iupmatrix {numcol=2, numlin=3,numcol_visible=2, numlin_visible=3,
                 height=34,width=34}

mat:setcell(0,0,"Inflation")
mat:setcell(1,0,"Medicine")
mat:setcell(2,0,"Food")
mat:setcell(3,0,"Energy")
mat:setcell(0,1,"January 2000")
mat:setcell(0,2,"February 2000")
mat:setcell(1,1,"5.6")
mat:setcell(2,1,"2.2")
mat:setcell(3,1,"7.2")
mat:setcell(1,2,"4.6")
mat:setcell(2,2,"1.3")
mat:setcell(3,2,"1.4")

mat.enteritem = function(self, l, c)
  print("entercell:", l,c)
end

dlg = iupdialog{mat}
dlg:showxy(IUP_CENTER,IUP_CENTER)
