
cv = iup.canvas{size="200x200"}

dg = iup.dialog{cv}
dg:show()

cv.keypress_cb = function(self, code, press) 
  print("keypress_cb("..code..", "..press..")")
end

cv.k_any = function(self, code) 
  print("k_any("..code..")")
end
