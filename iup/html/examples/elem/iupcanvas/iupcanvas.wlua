--IupCanvas Example in IupLua 

require( "iuplua" )

cv       = iup.canvas {size="300x100", xmin=0, xmax=99, posx=0, dx=10}
dg       = iup.dialog{iup.frame{cv}; title="IupCanvas"}

function cv:motion_cb(x, y, r)
  print(x, y, r)
  return iup.DEFAULT
end

dg:showxy(iup.CENTER, iup.CENTER)

iup.MainLoop()
