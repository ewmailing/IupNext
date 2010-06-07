require( "iuplua" )
require( "iupluacontrols" )

mat= iup.matrix{numlin=3, numcol=3}
mat:setcell(1,1,"Only numbers")
iup.maskMatSet(mat, "/d*", 0, 1, 1, 1) ;
dg = iup.dialog{mat}
dg:show()

if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
