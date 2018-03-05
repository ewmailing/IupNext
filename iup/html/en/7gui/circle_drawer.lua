require("iuplua")

opType = { }
opType.create = 1
opType.remove = 2
opType.update = 3

operations = {}
currentOperationId = 0
operationsCount = 0

function op_add(operation, id, x, y, radius)
	local opr= {}
	if currentOperationId < operationsCount then
		operationsCount = currentOperationId
	end
	opr.operation = operation
	opr.id = id
	opr.x = x
	opr.y = y
	opr.radius = radius
	table.insert(operations, currentOperationId+1, opr)
	operationsCount = operationsCount + 1
	currentOperationId = operationsCount
end

function op_has_undo()
	if currentOperationId < 1 then
		return nil
	end
	return 1
end

function op_has_redo()
	if currentOperationId == operationsCount then
		return nil
	end
	return 1
end

function op_undo()
	if currentOperationId < 1 then
		return
	end
	local operation = operations[currentOperationId]
	if  operation.operation == opType.create then
		circles_remove(operation.id)
	elseif operation.operation == opType.remove then
		circles_update(operation.id, operation.x, operation.y, operation.radius)
	else
		circles_update(operation.id, operation.x, operation.y, operation.radius)
	end
	currentOperationId = currentOperationId - 1
end

function op_redo()
	if currentOperationId == operationsCount then
		return
	end
	currentOperationId = currentOperationId + 1
	local operation = operations[currentOperationId]
	if  operation.operation == opType.create then
		circles_add(operation.x, operation.y, operation.id)
	elseif operation.operation == opType.remove then
		circles_update(operation.id, operation.x, operation.y, operation.radius)
	else
		circles_update(operation.id, operation.x, operation.y, operation.radius)
	end
end

DEF_RADIUS = 30

circles = {}
lastId = 0

function circles_add(x, y, id)
	local circle = {}
	circle.x = x
	circle.y = y
	circle.radius = DEF_RADIUS
	local validId
	if not id then
		lastId = lastId + 1
		validId = lastId
	else
		validId = id
	end
	circle.id = validId
	circles[validId] = circle
	return lastId
end

function circles_find(id)
	return circles[id].x, circles[id].y, circles[id].radius
end

function circles_remove(id)
	circles[id] = nil
end

function circles_update(id, x, y, radius)
	circles[id].x = x
	circles[id].y = y
	circles[id].radius = radius
end

function circles_pick(x, y)
	local i, dist, dx, dy, k, v
	local selectedCircle = -1
	for k, v in pairs(circles) do
		local dx = v.x - x
		local dy = v.y - y
		local d = ((dx*dx) + (dy*dy))^.5
		if  d < v.radius then
			if  selectedCircle < 0 then
				selectedCircle = v.id
				dist = d
			elseif d < dist then
				selectedCircle = v.id
			end
		end
	end
	return selectedCircle
end


--********************************** Main *****************************************

undoButton = iup.button{title = "Undo", size = "60", active = "NO"}
redoButton = iup.button{title = "Redo", size = "60", active = "NO"}

canvas = iup.canvas{expand = "YES", highlightedCircle = -1}

hbox = iup.hbox{iup.fill{}, undoButton, redoButton, iup.fill{}}

vbox = iup.vbox{hbox, canvas;  nmargin = "10x10", gap = "10"}

dlg = iup.dialog{vbox, title = "Circle Drawer", size = "300x150"}

val = iup.val{"HORIZONTAL"; expand = "HORIZONTAL", min = "0.5", max = "1.5", value = "1"}

config_box = iup.vbox{iup.fill{}, val, iup.fill{}, alignment = "ACENTER", nmargin = "10x10"}

config_dial = iup.dialog{config_box; size = "100x60"}

function undoButton:action()
	op_undo()
	if op_has_undo() then
		undoButton.active = "YES"
	else
		undoButton.active = "NO"
	end
	if op_has_redo() then
		redoButton.active = "YES"
	else
		redoButton.active = "NO"
	end
	iup.Update(canvas);
	return iup.default
end

function redoButton:action()
	op_redo()
	if op_has_undo() then
		undoButton.active = "YES"
	else
		undoButton.active = "NO"
	end
	if op_has_redo() then
		redoButton.active = "YES"
	else
		redoButton.active = "NO"
	end
	iup.Update(canvas);
	return iup.default
end

function drawCircles(canvas)
	local i, w, h

	iup.DrawBegin(canvas)

	w, h = iup.DrawGetSize(canvas)

	iup.SetAttribute(canvas, "DRAWCOLOR", "255 255 255")
	iup.SetAttribute(canvas, "DRAWSTYLE", "FILL")
	iup.DrawRectangle(canvas, 0, 0, w - 1, h - 1)

	iup.SetAttribute(canvas, "DRAWCOLOR", "0 0 0")
	iup.SetAttribute(canvas, "DRAWSTYLE", "STROKE")
	canvas["DRAWCOLOR"] = "0 0 0"
	canvas["DRAWSTYLE"] = "STROKE"

	for k, v in pairs(circles) do
		local circleId = v.id
		if  tonumber(canvas.highlightedCircle) == circleId then
			iup.SetAttribute(canvas, "DRAWCOLOR", "128 128 128")
			iup.SetAttribute(canvas, "DRAWSTYLE", "FILL")
		else
			iup.SetAttribute(canvas, "DRAWCOLOR", "0 0 0")
			iup.SetAttribute(canvas, "DRAWSTYLE", "STROKE")
		end

		iup.DrawArc(canvas, v.x - v.radius, v.y - v.radius,
					v.x + v.radius, v.y + v.radius, 0., 360.)
	end

	iup.DrawEnd(canvas)
end

function canvas:action(posx, posy)
	drawCircles(self)
	return iup.default
end

function canvas:button_cb(button, pressed, x, y, status)
	local id
	if  button == iup.BUTTON1 and pressed == 1 then
		id = circles_add(x, y)
		op_add(opType.create, id, x, y, DEF_RADIUS)

		if op_has_undo() then
			undoButton.active = "YES"
		else
			undoButton.active = "NO"
		end

		if op_has_redo() then
			redoButton.active = "YES"
		else
			redoButton.active = "NO"
		end
	
	elseif  button == iup.BUTTON3 and pressed == 1 then
		id = circles_pick(x, y)
		if id >= 0 then
			config_dial.circleId = id
			iup.Popup(config_dial, iup.CENTER, iup.CENTER)
			self.highlightedCircle = -1
		end
	end
	iup.Update(self)
	return iup.default
end

function canvas:motion_cb(x, y, status)
	self.highlightedCircle = circles_pick(x, y)
	iup.Update(self)
	return iup.default
end

function val:valuechanged_cb()
	local dial = iup.GetDialog(self)
	local value = self.value
	local circleId = tonumber(dial.circleId)
	local x, y, radius = circles_find(circleId)
	circles_update(circleId, x, y, DEF_RADIUS*value)
	op_add(opType.update, circleId, x, y, radius)

	if op_has_undo() then
		undoButton.active = "YES"
	else
		undoButton.active = "NO"
	end

	if op_has_redo() then
		redoButton.active = "YES"
	else
		redoButton.active = "NO"
	end

	iup.Update(canvas)
	
	return iup.default
end

dlg:showxy( iup.CENTER, iup.CENTER )

if (iup.MainLoopLevel()==0) then
	iup.MainLoop()
end
