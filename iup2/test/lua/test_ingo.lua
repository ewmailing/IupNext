dlg = iup.dialog {
  iup.vbox{
    iup.hbox{
      iup.vbox{
        iup.list{"AAA", "BBB", "CCCC"},
        iup.list{"AAA", "BBB", "CCCC"}
      },
      iup.vbox{
        iup.multiline{},
        iup.multiline{}
      }
    },
    iup.button{title = "Button", expand = "HORIZONTAL"}
  };
  title = "Dialog"
}

dlg:show()
