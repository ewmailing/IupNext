-- IupMask Example in Lua
-- Creates an IupText that accepts only numbers.

require( "iuplua" )
require( "iupluacontrols" )

txt = iup.text{}
iup.maskSet(txt, "/d*", 0, 1) ;
dg = iup.dialog{txt}
dg:show()

iup.MainLoop()
