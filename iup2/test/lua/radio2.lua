
function f( self, i )
  print("action " .. self.title .." ".. i)
end

t1 = iuptoggle{ title = "teste1", action = f }
t2 = iuptoggle{ title = "teste2", action = f }

rd = iupradio{ iupvbox{ t1,t2 }; value = t1 }

dl = iupdialog{ iuphbox{ iupfill{}, rd, iupfill{} } }

IupMap( dl )

rd.value = t2  -- SE COMENTAR AQUI, A CALLBACK NAO E' CHAMADA NO CASO CITADO

IupPopup( dl, IUP_CENTER, IUP_CENTER )
