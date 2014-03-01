-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local utils = import("utils")

local ui = Engine.ui

local _DefaultSort = function (self, cmp)
	if not cmp then
		cmp = function (a,b) return a.data[col] <= b.data[col] end
	end
	col = self.sortCol
	self.table = utils.stable_sort(self.table, cmp)
end

local _DefaultCellWidget = function (data)
	return ui:Label(data) end

local SmartTable = {}

function SmartTable.New (rowspec)
	local self = {}

	self.widget = ui:Table()
		:SetHeadingFont("LARGE")
		:SetColumnAlignment("JUSTIFY")
		:SetRowSpacing(10)

	self.table = {} -- data and widgets
	self.sortCol = nil
	self.defaultSortFunction = _DefaultSort
	self.sortFunction = _DefaultSort
	self.defaultCellWidget = _DefaultCellWidget

	setmetatable(self, {
		__index = SmartTable,
		class = "UI.SmartTable",
	})

	return self
end

function SmartTable.SetHeaders (self, headers)
	local row = {}
	for i,header in ipairs(headers) do
		local label = ui:Label(header)
		label.onClick:Connect(function () self:Sort(i) end)
		table.insert(row, label)
	end
	self.widget:SetHeadingRow(row)
end

function SmartTable.AddRow (self, cells)
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
end

function SmartTable.Sort (self, col)
	if col then
		self.sortCol = col
		self:sortFunction()
	end
	self:UpdateBody()
end

function SmartTable.UpdateBody (self)
	self.widget:ClearRows()
	for _,row in ipairs(self.table) do
		self.widget:AddRow(row.widgets)
	end
end

function SmartTable.Clear (self)
	self.table = {}
	self.widget:ClearRows()
end

function SmartTable.SetSortFunction (self, f)
	self.sortFunction = f
end

return SmartTable
