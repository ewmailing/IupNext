--IupCanvas Example in IupLua 

require( "iuplua" )

cv       = iup.canvas {size="300x100", xmin=0, xmax=99, posx=0, dx=10}
dg       = iup.dialog{iup.frame{cv}; title="IupCanvas"}

function cv:motion_cb(x, y, r)
  print(x, y, r)
end

function cv:k_any(c)
  print("c              = ", c)
  print("  XkeyBase(c)  = ", iup.XkeyBase(c))
  print("  isCtrlXkey(c)= ", iup.isCtrlXkey(c))
  print("  isAltXkey(c) = ", iup.isAltXkey(c))
end

dg:showxy(iup.CENTER, iup.CENTER)

if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
