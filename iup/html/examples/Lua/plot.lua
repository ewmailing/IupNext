
require("iuplua")
require("iuplua_plot")
require("cdlua")

plot = iup.plot{
--    TITLE = "Plot Test",
    MARGINBOTTOM = 20,
}

--iup.PlotBegin(plot, 0)
--iup.PlotAdd(plot, 0, 0)
--iup.PlotAdd(plot, 1, 1)
--iup.PlotEnd(plot)
plot:Begin(0)
plot:Add(0, 0)
plot:Add(1, 1)
plot:End()

function plot:predraw_cb(canvas)
  canvas:Foreground(cd.EncodeColor(0, 0, 255))
  canvas:Rect(200, 700, 200, 700) -- pixels
  canvas:Line(200, 700, 700, 200) -- pixels

  local x1, y1 = iup.PlotTransform(plot, 1, 0.9) -- plot coordinates
  local x2, y2 = iup.PlotTransform(plot, 1, 0.1)
  canvas:Line(x1, y1, x1, y2) -- pixels
end

dlg = iup.dialog{
    TITLE = "Plot Test",
    SIZE = "400x200",
    plot,
}

dlg:showxy(iup.CENTER, iup.CENTER)

iup.MainLoop()
