
local a= iup.vbox {iup.text {}; tabtitle= "A"}
local b= iup.vbox {iup.text {}; tabtitle= "B"}
local c= iup.vbox {iup.text {}, iup.frame{iup.canvas{size = "20x20", bgcolor="255 0 0"}}; tabtitle= "C"}

tab= iup.tabs
{
  a,
  b,
  c;
  size= "300x300",
  --expand= "NO",
}

dlg = iup.dialog
{
	iup.vbox
	{
		-- Qqr elemento a ser testado
		tab,
		
		iup.frame
		{
			iup.hbox
			{
				iup.fill {},
				iup.button { title = "   Fechar   " , action = "return iup.CLOSE" , expand = IUP_NO },
				iup.fill {},
			}
			;
			margin = "60x10",
			expand = IUP_HORIZONTAL
		}
	}
	;
	title = "TESTE",
	margin = "2x2",
	gap = 2
}

dlg:show()

