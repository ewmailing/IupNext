function f( self )        
  if( s1.visible == "YES" ) then
    s1.visible    = "NO"
    s1.old_rastersize = s1.rastersize  -- so´ para ficar mais interessante o exemplo
    m1.rastersize = "1x1"
    s1.rastersize = "1x1"
  else
    m1.rastersize = nil
    s1.rastersize = s1.old_rastersize
    s1.visible    = "YES"
  end
end

c1 = iupcanvas{ BGCOLOR = "0 0 0", SIZE = "100x100" }
b1 = iupbutton{ title = "Troca", action = f }
m1 = iupmultiline{ SIZE = "100x50", expand = "YES" }  -- o multiline tem que ter o expand=yes
s1 = iupsbox{ m1 }
s1.direction = "NORTH" -- faltou colocar a direcao certa

d1 = iupdialog
     { 
       iupvbox
       { 
          b1, 
          c1, 
          s1
       }
     }

d1 : show( IUP_CENTER, IUP_CENTER ); 
c1.size = nil -- para que voce possa reduzir o tamanho para um valor menor que o inicial.

