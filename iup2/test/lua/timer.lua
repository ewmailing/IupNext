local timer = iup.timer{
  time = 5,
  action_cb=function(self)
--self.run = "NO"
    print("timer called"..false)
  end,
}


local btstart = iup.button{
  title="start",
  action=function()
    print("timer started")
    timer.run = "YES"
  end
}


local btstop = iup.button{
  title="stop",
  action=function()
    print("timer stopped")
    timer.run = "NO"
  end
}


local dlg = iup.dialog{
  iup.hbox{
    btstart,
    btstop,
  };
  title = "timer bug"
}
dlg:popup(iup.CENTER,iup.CENTER) 