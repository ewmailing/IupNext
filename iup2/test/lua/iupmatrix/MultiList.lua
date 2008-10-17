

SEL_COLOR   = "255 0 0"
UNSEL_COLOR = "127 127 127"

MULTILIST = {}
MULTILIST_TAG = newtag()

function MULTILIST.SimpleMultiList(attrs)
 
	local mat = iupmatrix(attrs)
	
	-- Esses sao atributos obrigatorios do Multilist, vamos reforca-los aqui
	mat.mark_mode = "LIN" 
	mat.multiple  = IUP_NO
  mat.width1    = 3
  mat["fgcolor*:1"] = UNSEL_COLOR
  mat["bgcolor*:1"] = UNSEL_COLOR
  
  -- Primeiro marcado como default
  mat.marked = "L010"
  mat["fgcolor1:1"] = SEL_COLOR
  mat["bgcolor1:1"] = SEL_COLOR
  mat.last = "1:1"
	
	-- Callbacks obrigatorias
	mat.edition = function(self, lin, col, modo)
	  return IUP_IGNORE
	end

	mat.enteritem = function(self, lin, col, modo)
		if col == 1 then
			local str = "L"
			local i = 0

			self["bgcolor"..self.last] = UNSEL_COLOR
  		self["fgcolor"..self.last] = UNSEL_COLOR
			
			self.last = tostring(lin)..":"..tostring(col)
			
			while i < tonumber(self.numlin) do
				if i+1 == lin then
					str = str.."1"
				else
					str = str.."0"
				end
				i = i + 1
			end
			
			self.marked = str
		else 
		  self.focus_cell = lin..":"..1
		end
		
		self["bgcolor"..lin..":1"] = SEL_COLOR
		self["fgcolor"..lin..":1"] = SEL_COLOR
		
	end

  return mat  -- retorna a matrix

end

MULTILIST.GetIndex = function(table, index)
  local v = nil
  v = rawgettable(table, index)
  if v == nil then
    v = rawgettable(table.parent, index)
  end
  return v
end

function MULTILIST:setline(lin, t_line)
  local i = 1
  while i <= getn(t_line) do
    self:getmatrix():setcell(lin, i+1, t_line[i])
    i = i + 1
  end
end

function MULTILIST:setonecol(t)
  local i = 1
  while i <= getn(t) do
    self:getmatrix():setcell(i, 2, t[i])
    i = i + 1
  end
end

function MULTILIST:getline(lin)
  local i = 1
  local t = {}
  local mat = self:getmatrix()
  while i <= tonumber(mat.numcol) do
    t[i] = self:getmatrix():getcell(lin, i+1)
    i = i + 1
  end
  return t
end

function MULTILIST:getmatrix()
  return self.matrix
end

function MultiList(lin, col, lin_vis, col_vis)
  local matrix
  local attrs = {}
  local obj   = {}

  attrs.numlin         = lin
  attrs.numcol         = col + 1
  attrs.numlin_visible = lin_vis
  attrs.numcol_visible = col_vis + 1
  attrs.expand         = IUP_YES
  attrs["BGCOLOR*:1"]  = "200 200 200"

  matrix       = MULTILIST.SimpleMultiList(attrs)
  obj.matrix   = matrix

  obj.parent = MULTILIST
  -- Setando a tag para o objeto
  settag(obj, MULTILIST_TAG);

  return obj
end

settagmethod(MULTILIST_TAG, "index", MULTILIST.GetIndex)
