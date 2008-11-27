-- Creates an IupText that accepts only numbers.

require( "iuplua" )
require( "iupluacontrols" )

txt = iup.text{}
txt.MASK = "/d*"
dg = iup.dialog{txt;title="MASK Example"}
dg:show()

iup.MainLoop()
