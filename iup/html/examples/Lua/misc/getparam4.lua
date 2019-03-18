require( "iuplua" )
require( "iupluacontrols" )

res, inf,outf = iup.GetParam("Title", nil,[[
Input File %f[,,,]
Output File %f[,,,]
	]],"*","*")
	
if res then	
	iup.Message("Files",inf .. outf)
end


	