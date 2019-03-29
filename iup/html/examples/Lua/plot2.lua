
require("iuplua")
require("iuplua_plot")
require("cdlua")

plot = iup.plot{
--    TITLE = "Plot Test",
    MARGINBOTTOM = 20,
}

plot:Begin(0)
plot:Add(0, 0)
plot:Add(1, 1)
plot:End()

plot:Begin(0)
plot:End()

local points_x = {0.1, 0.5, 0.7}
local points_y = {0.1, 0.5, 0.1}
plot:AddSamples(1,points_x,points_y,#points_x)

dlg = iup.dialog{
    TITLE = "Plot Test",
    SIZE = "400x200",
    plot,
}

dlg:showxy(iup.CENTER, iup.CENTER)

iup.MainLoop()
