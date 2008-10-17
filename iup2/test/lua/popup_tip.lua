d = iup.dialog{iup.button{title="a", tip="aaaa",
action=function()
 local e = iup.dialog{iup.button{title="b", tip="bbbb"}}
 e:popup(200,200)
 e:destroy()
end
}}
d:popup(100,100)
d:destroy()

collectgarbage("count")
print(gcinfo())
