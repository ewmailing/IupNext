
t = iuptimer{}

t.time = 1000

t.action = function(self)
  print(self, "timeup")
end

t.run = "YES"
iupdialog{iuplabel{title="Timer Test"}}:show()

