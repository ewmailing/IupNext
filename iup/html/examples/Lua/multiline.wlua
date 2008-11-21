--  IupMultiline Simple Example in IupLua 
--  Shows a multiline that ignores the treatment of the DEL key, canceling its effect. 

require( "iuplua" )

ml = iup.multiline{expand="YES", value="I ignore the DEL key!", border="YES"}

ml.action = function(self, c, after)
   if c == iup.K_DEL then
     return iup.IGNORE
  else
    return iup.DEFAULT;
  end
end

dlg = iup.dialog{ml; title="IupMultiline", size="QUARTERxQUARTER"}
dlg:show()

iup.MainLoop()
