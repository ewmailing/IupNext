
dofile("MultiList.lua")

--===============================
--Início da aplicação

ml = MultiList(20, 1, 5, 1)

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


ml:setonecol{"Marcos", "Mark", "Mauricio", "Jose", "Joao"} -- Dado lido direto do BD

box = iupframe{ml:getmatrix()}

dg = iupdialog{box; title="TESTE DO MULTILIST"}
dg:show()

IupSetFocus(ml:getmatrix())
