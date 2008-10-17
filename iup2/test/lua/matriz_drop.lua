mat = iup.matrix
{
   numcol=2,
   numlin=4,
   numcol_visible=2,
   numlin_visible=4,
   widthdef=34,
   drop = function(self, drop, lin, col)
      if(lin == 3) then
         IupSetAttribute( drop, 1, "A")
         IupSetAttribute( drop, 2, "B" )
         IupSetAttribute( drop, 3, "C" )
         drop.value = 1
         return IUP_DEFAULT
      end
      return IUP_IGNORE
   end
}



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
dlg = iup.dialog{iup.vbox{mat; margin="10x10"}}
dlg:showxy(iup.CENTER, iup.CENTER)