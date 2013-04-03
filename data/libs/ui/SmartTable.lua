local ui = Engine.ui

_DefaultSort = function (self)
	col = self.sortCol
	table.sort(self.table, function (a,b) 
		return a.data[col] < b.data[col] end
	)
end

UI.SmartTable = {

New = function (rowspec)
	local self = {}
	
	self.rowspec = rowspec
	self.headers = ui:Grid(rowspec,1)
	self.body = ui:VBox(10) 
	
	self.table = {} -- data and widgets
	self.sortCol = nil
	self.defaultSortFunction = _DefaultSort
	self.sortFunction = self.defaultSortFunction

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

AddRow = function (self, rowData, rowWidgets)
	table.insert(self.table, {data = rowData, widgets = rowWidgets})
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
	for i,row in ipairs(self.table) do
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
