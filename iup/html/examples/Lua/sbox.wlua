require( "iuplua" )
require( "iupluacontrols" )

bt = iup.button{title="Test"}
bt.expand = "YES"
box = iup.sbox{bt}
box.direction = "SOUTH"
box.color = "0 0 255"

ml = iup.multiline{}
ml.expand = "YES"
vbox = iup.vbox{box, ml}

lb = iup.label{title="Label"}
lb.expand = "YES"
dg = iup.dialog{iup.hbox{vbox, lb}}
dg:show()

if (not iup.MainLoopLevel or iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
