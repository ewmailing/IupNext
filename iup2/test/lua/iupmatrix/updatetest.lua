local function gethue(index)
	return math.mod(math.floor((index-1)/4), 2),
	       math.mod(math.floor((index-1)/2), 2),
	       math.mod((index-1), 2)
end

local function bgcolor_cb(self, lin, col)
	if lin == 0 then
		return iup.IGNORE
	elseif col == 0 then
		return iup.IGNORE
	else
		local bright = 255*col/8
		local r,g,b = gethue(lin)
		return r * bright,
		       g * bright,
		       b * bright,
		       iup.DEFAULT
	end
end

local function fgcolor_cb(self, lin, col)
	if lin == 0 then
		return iup.IGNORE
	elseif col == 0 then
		return iup.IGNORE
	else
		local bright = 255*((8-col)/8)
		local r,g,b = gethue(lin)
		return r * bright,
		       g * bright,
		       b * bright,
		       iup.DEFAULT
	end
end

local function value_cb(self, lin, col)
	if lin == 0 then
		return iup.IGNORE
	elseif col == 0 then
		return iup.IGNORE
	else
		return table.concat({gethue(lin)}, " ")
	end
end

local function edition_cb() return iup.IGNORE end

local mat = iup.matrix {
	numcol=8,
	numlin=10,
	numcol_visible=2,
	numlin_visible=3,
	widthdef=34,
	
	bgcolor_cb = bgcolor_cb,
	fgcolor_cb = fgcolor_cb,
	value_cb   = value_cb,
	edition_cb = edition_cb,
}

local counter = 0
local function action_cb()
	counter = counter + 1
	mat.numlin = math.mod(counter, 16)
	mat.redraw = "ALL"
	return iup.DEFAULT
end

local timer = iup.timer {
	time = 3000,
	action_cb = action_cb,
}
--timer.run = "YES"

local dlg = iup.dialog {
	iup.hbox {
		mat,
		iup.vbox {
			iup.button {
				title = "Start",
				action = function()
					timer.run = "YES"
				end,
			},
			iup.button {
				title = "Stop",
				action = function()
					timer.run = "NO"
				end,
			},
			iup.button {
				title = "Clean",
				action = function()
					mat.numlin = 0
					mat.redraw = "ALL"
				end,
			},
		},
	}
	;
  rastersize = "750x550",
}
dlg:show(iup.CENTER, iup.CENTER)