l = iuplabel{title="labell"}
b = iupbutton{title="buttonn"}
v = iupvbox{l, b}
d = iupdialog{v}
d:show()

tmp = IupGetNextChild(v, nil)
print("1) label? ", tmp, IupGetType(tmp))
tmp = IupGetNextChild(v, l)
print("2) button?", tmp, IupGetType(tmp))
tmp = IupGetBrother(l)
print("3) button?", tmp, IupGetType(tmp))
