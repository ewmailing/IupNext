m = iup.multiline{size="200x200"}
b = iup.button{title="Go"}
function b:action() 
  self.caret = "10,10" 
end
dg = iup.dialog{iup.vbox{m,b}}
dg:show()
