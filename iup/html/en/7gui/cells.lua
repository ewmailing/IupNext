require("iuplua")

data = {}
formula = {}
dependencies = {}

function getValue(lin, col)
	local value = 0.0
	if data[lin] and data[lin][col] then
		value = data[lin][col]
	end
	return value
end

function setValue(lin, col, value)
	if not data[lin]then
		data[lin] = {}
	end
	data[lin][col] = value
	return 0.0
end

function sumCells(statLin, startCol, endLin, endCol)
	local total = 0
	for i = statLin, endLin do
		for j = startCol, endCol do
			total += data[i][j]
		end
	end
	return total
end

function runFormula(lin, col)
	local op, sc, sl, ec, el = string.match(formula[lin][col], "^(%u+)%((%u)(%d):(%u)(%d)%)$")
  if op and sc and sl and ec and el then
    if op == "SUM" then
        data[lin][col] = sumCells(slin, scol, elin, endcol)
    end
  end
end

--********************************** Main *****************************************

matrix = iup.matrix{numcol = 26, numlin = 100, numcol_visible = 4, numlin_visible = 7,
					width0 = 20, height0 = 8, alignment0 = "ACENTER", scroolbar = "YES"}

dlg = iup.dialog{iup.hbox{matrix}; title = "Cells", size = "300x150"}

function matrix:value_cb(lin, col)
	local text
	local editcell = self.editcell
	if editcell then
		local elin, ecol
		elin, ecol = string.match(editcell, "(%d):(%d)")
		if elin == lin and ecol == col and isFormula(lin, col) then
		  return getFormula(lin, col);
		end
	end
	if lin == 0 and col == 0 then
		return ""
	elseif lin == 0 then
		return string.format("%c", string.byte('A') + col - 1)
	elseif col == 0 then
		return string.format("%d", lin - 1)
	end
	local value = getValue(lin, col)
	if not isnumber(value) then
		text = value
	else
		text = string.format("%.2f", value)
	end
	return text
end

function matrix:value_edit_cb(lin, col, newValue)
	if string.find(newValue, "^[+-]?%d+$") then
		setValue(lin, col, newvalue)
		for k, v in pairs(dependencies[lin][col]) do
			runFormula(v.lin, v.col)
		end
	else
		formula[lin][col] = newvalue
		runFormula(lin, col)
	end
	return iup.DEFAULT
end

dlg:showxy( iup.CENTER, iup.CENTER )

if (iup.MainLoopLevel()==0) then
  iup.MainLoop()
end
