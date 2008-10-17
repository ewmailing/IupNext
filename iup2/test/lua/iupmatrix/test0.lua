
dofile("MultiList.lua")

--===============================
--Início da aplicação

ml = MultiList(3, 3, 3, 3)

matrix = ml:getmatrix()
function matrix:action_cb(c,lin)
  if c==K_CR then
    local i = 1
    local t = ml:getline(lin)
    while i <= getn(t) do
      print(t[i])
      i = i + 1
    end
  end
end


ml:setline(1, {"teste1", "teste2"})
ml:setline(2, {"aaaaaa", "bbbbbb"})
ml:setline(3, {"xxxxxx", "yyyyyy"})

box = iupframe{ml:getmatrix()}

dg = iupdialog{box; title="TESTE DO MULTILIST"}
dg:show()

IupSetFocus(ml:getmatrix())
