
$debug

dofile("TableTree.lua")

t = {}
t.name  = "Joao"
t.work  = {}
t.work.tel  = "09877890"
t.work.addr = "No importance Street"
t.personal = {}
t.personal.tel = "929121291"
t.personal.addr = "No name Street"


TableTree(t)
