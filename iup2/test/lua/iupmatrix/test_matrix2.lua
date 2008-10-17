matrix = iup.matrix{
          HEIGTH0 = 10, numcol = 3 , numlin = 8 , numcol_visible = 3 , 
          numlin_visible = 8,
          width1 = 70 , width2 = 50 , width3 = 70 , click_cb = click,
          alignment1 = "ACENTER" , alignment2 = "ACENTER" , 
          alignment3 = "ACENTER",
          edition_cb = edition_cb
}


matrix:setcell(0,1,"Camada")
matrix:setcell(0,2,"Visível")
matrix:setcell(0,3,"Afastamento\npara K-1")


for i=1,matrix.numlin do
   iup.maskMatSetFloat (matrix,0,0,100000000000,i,3)
end 
