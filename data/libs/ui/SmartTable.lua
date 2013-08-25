-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui

_DefaultSort = function (self, cmp)
	if not cmp then
		cmp = function (a,b) return a.data[col] <= b.data[col] end
	end
	col = self.sortCol
	self.table = stable_sort(self.table, cmp)
end

_DefaultCellWidget = function (data)
	return ui:Label(data) end

UI.SmartTable = {

New = function (rowspec)
	local self = {}
	
	self.rowspec = rowspec
	self.headers = ui:Grid(rowspec,1)
	self.body = ui:VBox(10) 
	
	self.table = {} -- data and widgets
	self.sortCol = nil
	self.defaultSortFunction = _DefaultSort
	self.sortFunction = _DefaultSort
	self.defaultCellWidget = _DefaultCellWidget

	self.widget =
		ui:VBox(10):PackEnd({
			self.headers,
			ui:Scroller():SetInnerWidget(self.body)
		})

	setmetatable(self, {
		__index = UI.SmartTable,
		class = "UI.SmartTable",
	})

	return self
end,

SetHeaders = function (self, headers)
	for i,header in ipairs(headers) do
		local label = ui:Label(header)
		label.onClick:Connect(function () self:Sort(i) end)
		self.headers:SetCell(i-1, 0, label)
	end
end,

AddRow = function (self, cells)
	local row = {data = {}, widgets = {}}
	for _,cell in ipairs(cells) do
		if not cell.widget then  -- if widget isn't specified use default one
			cell.widget = self.defaultCellWidget(cell.data)
		end
		table.insert(row.data, cell.data)
		table.insert(row.widgets, cell.widget)
	end
	table.insert(self.table, row)
	self:Sort(self.sortCol)
end,

Sort = function (self, col)
	if col then
		self.sortCol = col
		self:sortFunction()
	end
	self:UpdateBody()
end,

UpdateBody = function (self)
	self.body:Clear()
	for _,row in ipairs(self.table) do
		local rowGrid = ui:Grid(self.rowspec, 1)
		rowGrid:SetRow(0, row.widgets)
		self.body:PackEnd(rowGrid)
	end
end,

Clear = function (self)
	self.table = {}
	self.body:Clear()
end,

SetSortFunction = function (self, f)
	self.sortFunction = f
end,
}
