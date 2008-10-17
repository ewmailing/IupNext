t = iupspinbox{iuptext{}}
d = iupdialog{t}
d:show()

t.spincb = function(self, i)
  print(i)
end
