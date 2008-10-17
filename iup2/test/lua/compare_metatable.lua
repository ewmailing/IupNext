
l = iuplist{}
b = iupbutton{title="xpto"}
d = iupdialog{iuphbox{l, b}}
d:show()
x = IupGetFocus()
print("x=", x, "b=", b)
if x == b then
  print("é igual")
else
  print("é diferente")
end

