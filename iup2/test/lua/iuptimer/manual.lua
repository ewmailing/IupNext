
timer_action = function(self)
  print("timer")
  if self == timer1 then
    print("timer 1 called")
  elseif self == timer2 then
    print("timer 2 called")
    return IUP_CLOSE
  end
  return IUP_DEFAULT
end

timer1 = iuptimer{action=timer_action, time=100}
timer2 = iuptimer{action=timer_action, time=2000}

timer1.run = "YES"
timer2.run = "YES"

dg = iupdialog{iuplabel{title="Timer example"}}
dg:show()

