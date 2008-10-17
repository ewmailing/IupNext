label = iuplabel{title="Possible commands are defined in xml1.xml"}
text  = iuptext {size="200"}

function reco(self, msg)
  text.value = msg
end

sk = iupspeech{action=reco, grammar="xml1.xml", say="xml1 loaded"}

dg = iupdialog{iupvbox{label, text}}
dg:show()