t = iuptext{action=function(a,b,c) print("Funcionou", a,b,c) end}
d = iupdialog{t}
d:show()
