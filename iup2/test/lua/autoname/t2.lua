tg1 = iuptoggle { title = "SIM" }
tg2 = iuptoggle { title = "NAO" }
tg3 = iuptoggle { title = "TALVEZ" }

radio = iupradio
{
	iupvbox
	{
		tg1,
		tg2,
		tg3,
	}
}

tg_cb = function (self, v)
	print ("radio.value: " , radio.value)
	print ("radio.value: " , radio.value.title)
end

tg1.action = tg_cb
tg2.action = tg_cb
tg3.action = tg_cb
 
dlg = iupdialog
{
	iupvbox
	{
		radio,
		iupframe
		{
			iuphbox
			{
				iupfill {},
				iupbutton { title = "   Fechar   " , action = "return IUP_CLOSE" , expand = IUP_NO },
				iupfill {},
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

