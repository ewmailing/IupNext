a = iup.LoadImage("tec-site.gif")
print(a, iup.GetType(a))
b = iup.label{title="shit", image=a}
d = iup.dialog{b}
d:show()
