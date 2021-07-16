local ui = require 'pigui.baseui'

--
-- Table: array_table
--

local array_table = {}

--
-- Function: array_table.draw
--
-- Draw a table based on the contents of the "tbl" array
--
-- > array_table.draw(id, tbl, iter, column, extra)
--
-- Parameters:
--
--   id - unique string identifier
--
--   tbl - an array of hashtables with the same keys - main data source
--
--   iter - pairs, ipairs, e.t.c.
--
--   columns - array of hashtables, determining the presence and order of columns. keys:
--     name - column name
--     key - key name from table
--     fnc - function to convert value to string
--     string - if true, the given column is sorted as a string, false - as number
--
--   extra
--     totals - table similar to tbl, its contents are shown at the very botton, under the separator
--     callbacks
--       onClick - this function is called if the line is clicked
--       isSelected - this function is called for every row and if it returns true the row is highlighed
--
-- Example:
--
-- > table_array.draw("hyperspace_params", params, ipairs,
-- > 	{
-- > 		{ name = "From",           key = "from",           fnc = sysName,             string = true },
-- > 		{ name = "Distance",       key = "distance",       fnc = format("%.2f l.y.")                },
-- > 		{ name = "Fuel",           key = "fuel",           fnc = format("%.2f t")                   },
-- > 		{ name = "Duration",       key = "duration",       fnc = ui.Format.Duration                 },
-- > 		{ name = "Cloud Duration", key = "cloud_duration", fnc = ui.Format.Duration                 }
-- > 	},{
-- > 		callbacks = {
-- > 			onClick = function(row)
-- > 				Game.sectorView:SwitchToPath(row.from)
-- > 			end,
-- > 			isSelected = function(row)
-- > 				return row.from:IsSameSystem(selected_in_sectorview)
-- > 			end
-- > 	}})

local sort_param = {} -- we need to keep sorting options between frames

array_table.draw = function(id, tbl, iter, columns, extra)
	local callbacks = extra and extra.callbacks
	local data = {}
	-- sort so that nil always goes down the table
	local function less   (a, b) return a and ( not b or a < b ) end
	local function greater(a, b) return a and ( not b or b < a ) end
	local sort_symbol = {
		[less]    = { sym = "▲ ", nxt = greater },
		[greater] = { sym = "▼ ", nxt = less    }
	}
	if not sort_param[id] then sort_param[id] = {} end

	-- build table
	for i, row0 in iter(tbl) do
		local row = {}
		for j,column_cfg in ipairs(columns) do
			if column_cfg.key == "#" then
				row[j] = i
			else
				row[j] = row0[column_cfg.key]
			end
		end
		-- remember the original row for callbacks
		row[#columns+1] = row0
		table.insert(data, row)
	end
	local widths = {}
	for i = 1, #columns do widths[i] = 0 end

	-- draw table
	-- top line
	ui.columns(#columns, id, false)
	for i, column_cfg in ipairs(columns) do
		if sort_param[id][column_cfg.key] then
			local key = column_cfg.key
			local fnc = sort_param[id][key]
			if column_cfg.string then
				local string_fnc = column_cfg.fnc or tostring
				table.sort(data, function(a, b)
					return fnc(a[i] and string_fnc(a[i]), b[i] and string_fnc(b[i]))
				end)
			else
				table.sort(data, function(a, b) return fnc(a[i], b[i]) end)
			end
			if ui.selectable(sort_symbol[fnc].sym .. column_cfg.name .. "##" .. id, false, {}) then
				sort_param[id] = { [column_cfg.key] = sort_symbol[fnc].nxt }
			end
		else
			if ui.selectable(column_cfg.name .. "##" .. id, false, {}) then
				sort_param[id] = { [column_cfg.key] = less }
			end
		end
		widths[i] = math.max(widths[i], ui.calcTextSize(sort_symbol[less].sym .. column_cfg.name .. "--").x)
		ui.nextColumn()
	end
	ui.separator()
	-- other lines
	local highlight_box
	local selected_boxes = {}
	local top_left, down_right = 0, 0
	-- draw next cell and recalculate column width
	local function putCell(item, i, fnc)
		if not fnc then fnc = tostring end
		local txt = item and fnc(item) or "-"
		ui.text(txt)
		widths[i] = math.max(widths[i], ui.calcTextSize(txt .. "--").x)
	end
	if #data > 0 then
		for _,item in ipairs(data) do
			top_left = ui.getCursorScreenPos()
			for i,_ in ipairs(columns) do
				putCell(item[i], i, columns[i].fnc)
				down_right = ui.getCursorScreenPos()
				down_right.x = down_right.x + ui.getColumnWidth()
				ui.nextColumn()
			end
			down_right.y = ui.getCursorScreenPos().y - 2
			top_left.y = top_left.y - 2
			if callbacks and callbacks.isSelected and callbacks.isSelected(item[#columns + 1]) then
				table.insert(selected_boxes, { top_left, down_right })
			end
			if not highlight_box and ui.isWindowHovered() and ui.isMouseHoveringRect(top_left, down_right, false) then
				highlight_box = { top_left, down_right }
				if ui.isMouseClicked(0) and callbacks and callbacks.onClick then
					callbacks.onClick( item[#columns + 1] ) -- in the additional column we have saved the original row
				end
			end
		end
	end
	-- totals
	if extra and extra.totals then
		ui.separator()
		for _, row in ipairs(extra.totals) do
			for i,column_cfg in ipairs(columns) do
				putCell(row[column_cfg.key], i, column_cfg.fnc)
				ui.nextColumn()
			end
		end
	end
	-- set column width
	for i = 1,#widths do ui.setColumnWidth(i-1, widths[i]) end
	-- end columns
	ui.columns(1, "")
	-- draw highlight
	if highlight_box then
		ui.addRectFilled(highlight_box[1], highlight_box[2], ui.theme.colors.white:opacity(0.3), 0, 0)
	end
	if #selected_boxes ~= 0 then
		for _, selected_box in ipairs(selected_boxes) do
			ui.addRectFilled(selected_box[1], selected_box[2], ui.theme.colors.white:opacity(0.2), 0, 0)
		end
	end
end

--
-- Function: array_table.addkeys
--
-- create stateless iterator to create "virtual" keys on the fly
-- the k,v from iterator is passed to function, and the value that it returns is written to given key
--
-- > myiter = addKeys(iter, config)
--
-- Parameters:
--
--  iter - pairs, ipairs e.t.c.
--
--  config - hashtable:
--    key1 = fnc1(k,v) ... return value1 end
--    key2 = fnc2(k,v) ... return value2 end
--    ...
--
-- Return:
--
--- Example:
--
-- > local a = { {one = 10, two = 20},
-- >             {one = 15, two = 18} }
-- > local addSum = addKeys(ipairs, { sum = function(k,v) return v.one + v.two end } )
-- >
-- > for k, v in addSum(a) do
-- >   print(k, v.one, v.two, v.sum)
-- > end
-- >
-- > 1, 10, 20, 30
-- > 2, 15, 18, 33
--
array_table.addKeys = function(iter, config)
	return function(tbl)
		local step, context, position = iter(tbl)
		return function(a, i)
			local row0
			i, row0 = step(a, i)
			if row0 then
				local row = {}
				if type(row0) == 'table' then
					for key, value in pairs(row0) do
						row[key] = value
					end
				else
					row[1] = row0
				end
				for key, fnc in pairs(config) do
					row[key] = fnc(i, row0)
				end
				return i, row
			end
		end, context, position
	end
end

return array_table
