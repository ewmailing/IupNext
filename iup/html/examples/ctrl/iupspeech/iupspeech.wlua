--IupSpeech Example in Lua
--NOTE: The documentation says this is not available in Lua.

require( "iuplua" )
require( "iupluacontrols" )

label = iup.label{title="Possible commands are defined in xml1.xml"}
text = iup.text {size="200"}

function reco(self, msg)
  text.value = msg
end

sk = iup.speech{action=reco, grammar="xml1.xml", say="xml1 loaded"}

dg = iup.dialog{iupvbox{label, text}; title = "IupSpeech Test"}
dg:show()

iup.MainLoop()
