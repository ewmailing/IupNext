box1 = iup.hbox
{
	iup.label { title = "Lista dropDown:" },
	iup.list { "Elem1" , "Elem2" , "Elem3" , "Elem4" ; dropdown = "YES" }
	;
	margin = "15x2"
}

box2 = iup.hbox
{
	iup.label { title = "Texto:" },
	iup.text { size = 120 }
	;
	margin = "15x2"
}

zbox = iup.zbox { box1 , box2 }

local frame = iup.frame
{
	iup.hbox
	{
		iup.fill {},
		zbox,
		iup.fill {}
	}
	;
	title = "ZBox",
	margin = "15x15",
	expand = "YES"
}

local dlg = iup.dialog
{
	iup.vbox
	{
		-- Qqr elemento a ser testado
		frame,
		
		iup.frame
		{
			iup.hbox
			{
				iup.fill {},
				iup.button { title = "   Fechar   " , action = "return iup.CLOSE" , expand = "NO" },
				iup.fill {},
			}
			;
			margin = "60x10",
			expand = "HORIZONTAL"
		}
	}
	;
	title = "TESTE",
	margin = "2x2",
	gap = 2
}

dlg:map()

dlg.size = nil
dlg:show()

--zbox.value = box1
--zbox.value = box2

