-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local theme = ui.theme

local defaultFuncs = {

    initTable = function(self)
    end,

    onMouseOverItem = function(self, item)
    end,

    onClickItem = function (self, e)

    end,

    renderItem = function(self, item)

    end,

    -- sort items in the market table
    sortingFunction = function(e1,e2)
        return e1 < e2
    end,

	iterator = pairs
}

local TableWidget = {}

function TableWidget.New(id, title, config)
    local defaultSizes = ui.rescaleUI({
        itemPadding = Vector2(14, 14),
        itemSpacing = Vector2(4, 9),
    }, Vector2(1600, 900))

    local self
    self = {
        id = id,
        popup = config.popup or ModalWindow.New('popupMsg' .. id, function()
            ui.text(self.popup.msg)
            ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
            ui.sameLine()
            if ui.button("OK", Vector2(100, 0)) then
                self.popup:close()
            end
        end),
        title = title,
        items = {},
        scrollReset = false,
        itemTypes = config.itemTypes or {},
        columnCount = config.columnCount or 0,
        style = {
            itemPadding = config.itemPadding or defaultSizes.itemPadding,
            itemSpacing = config.itemSpacing or defaultSizes.itemSpacing,
            size = config.size or Vector2(ui.screenWidth / 2,0),
            titleFont = config.titleFont or ui.fonts.orbiteer.xlarge,
            highlightColor = config.highlightColor or theme.colors.tableHighlight,
            selectionColor = config.selectionColor or theme.colors.tableSelection,
        },
        funcs = {
            initTable = config.initTable or defaultFuncs.initTable,
            renderHeaderRow = config.renderHeaderRow or false,
            canDisplayItem = config.canDisplayItem or defaultFuncs.canDisplayItem,
            renderItem = config.renderItem or defaultFuncs.renderItem,
            onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem,
            onClickItem = config.onClickItem or defaultFuncs.onClickItem,
            sortingFunction = config.sortingFunction or defaultFuncs.sortingFunction,
			iterator = config.iterator or defaultFuncs.iterator
        },
    }

    setmetatable(self, {
        __index = TableWidget,
        class = "UI.TableWidget",
    })

    return self
end

function TableWidget:render()
    ui.withStyleVars({ItemSpacing = self.style.itemSpacing}, function()
        ui.child("Table##" .. self.id, self.style.size, function()
            if self.scrollReset then
                ui.setScrollHereY()
                self.scrollReset = false
            end

            if(self.title) then
                ui.withFont(self.style.titleFont.name, self.style.titleFont.size, function()
                    ui.text(self.title)
                end)
			end

			if not self.selectedItem then self.selectionStart = nil end

            -- If highlightStart is set, the mouse hovered over an item in the previous frame, so draw a rectangle underneath for highlighting
            if self.highlightStart then ui.addRectFilled(self.highlightStart, self.highlightEnd, self.style.highlightColor, 0, 0) end
            if self.selectionStart then ui.addRectFilled(self.selectionStart, self.selectionEnd, self.style.selectionColor, 0, 0) end

            -- We're using self.columnCount+1 to help with calculating the bounds of the highling rect
            -- ui.getCursorPos() always returns the top-left corner of a column, so to get the max x-coordninate of
            -- a table we need to call it just before we move to the next row, and to get the max y-coordinate,
            -- we need to call it just after. An empty column is added so this logic can be handled here
            -- rather than in custom handlers like renderItem which will be redefined for every instance of the widget
            ui.columns(self.columnCount+1, self.id, false)
            self.funcs.initTable(self)

            if(self.funcs.renderHeaderRow) then
                self.funcs.renderHeaderRow(self)
                ui.nextColumn()
            end

            local startPos
            local endPos
            local selOffset = self.style.itemSpacing.y / 2

            self.highlightStart = nil
            self.highlightEnd = nil

            for key, item in self.funcs.iterator(self.items) do
				startPos = ui.getCursorScreenPos() - Vector2(4, selOffset)

				self.funcs.renderItem(self, item, key)

                endPos = ui.getCursorScreenPos()

                ui.nextColumn()

				-- center the selection
				-- endPos.y points to the beginning of the new row so to center the selection we also move it a bit higher
                endPos.y = ui.getCursorScreenPos().y - selOffset

                if ui.isWindowHovered() and ui.isMouseHoveringRect(startPos, endPos, false) then
                    self.funcs.onMouseOverItem(self, item, key)
                    if ui.isMouseClicked(0) then
                        self.funcs.onClickItem(self, item, key)
                    end

                    self.highlightStart = startPos
                    self.highlightEnd = endPos
				end

				if self.selectedItem == item then
					self.selectionStart = startPos
					self.selectionEnd = endPos
				end
            end

            ui.columns(1, "", false)
        end)
    end)
end

return TableWidget
