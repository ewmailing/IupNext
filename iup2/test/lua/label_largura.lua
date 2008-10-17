
local texto= iuplabel
{
  title= "aaaaaaaaaaa"
}
local bt1= iupbutton
{
  title= "Ok"
}

local cxVert= iupvbox
{
  texto,
  bt1,
}



local dlg= iupdialog
{
  cxVert,

  ;title= "teste label multilinha"
}

dlg:show ()


local ADICIONAL= "\naaaaaaaaaaa"

function bt1:action ()
  texto.title= texto.title .. ADICIONAL
  dlg.size= nil
end
