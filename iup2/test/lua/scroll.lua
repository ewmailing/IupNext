
function f( b )
  c.dy = c.ymax - c.ymin
  c.posy = c.ymin
  --c.scrollbar = "NO"
end

function p( b )
  c.dy = 0.1
  c.posy = c.ymin
  --c.scrollbar = "NO"
end

b = iupbutton{ title = "scroll no", action = f }
b2 = iupbutton{ title = "scroll yes", action = p }
c = iupcanvas{ size = "100x100", ymin=0, ymax=1, dy=0.1, scrollbar = "VERTICAL" }
d = iupdialog{ iupvbox{ c, b, b2 } }

d:map( )
d:popup( IUP_CENTER, IUP_CENTER )
