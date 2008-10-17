
test_canvas = function(self)
  print("canvas", self)
end

test_dialog = function(self)
  print("dialog", self)
end

cv = iupcanvas{mapcb = test_canvas}

dg = iupdialog{cv; mapcb = test_dialog}
dg:show()