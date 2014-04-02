-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")

local normalColor = { r=0.5, g=0.5, b=0.5, a=1.0 }
local hoverColor  = { r=0.8, g=0.8, b=0.8, a=1.0 }
local activeColor = { r=1.0, g=1.0, b=1.0, a=1.0 }

local ui = Engine.ui

local _FindTabNum = function (self, id)
	for i = 1,#self.tabs do
		if self.tabs[i].id == id then
			return i
		end
	end
	return nil
end

local TabView = {}

function TabView.New ()
	local self = {
		tabs   = {},
	}

	self.header    = ui:HBox(10)
	self.title     = ui:Label(""):SetFont("HEADING_XLARGE")
	self.titleArea = ui:Expand("HORIZONTAL"):SetInnerWidget(ui:Align("RIGHT"):SetInnerWidget(self.title))
	self.body      = ui:Expand()
	self.footer    = ui:Margin(0)

	self.widget =
		ui:VBox():PackEnd({
			ui:Margin(5):SetInnerWidget(
				ui:HBox():PackEnd(self.header)
			),
			ui:Margin(5):SetInnerWidget(
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({
						self.body,
						self.footer
					})
				)
			)
		})

	setmetatable(self, {
		__index = TabView,
		class = "UI.TabView",
	})

	return self
end

function TabView.AddTab (self, args)
	local id       = args.id
	local title    = args.title
	local icon     = args.icon
	local template = args.template

	local tab = {
		group    = self,
		id       = id,
		icon     = icon,
		title    = title,
		template = template,
	}

	self:RemoveTab(id)

	local num = #self.tabs+1
	self.tabs[num] = tab

	tab.iconWidget = ui:Icon(tab.icon):SetColor(normalColor)
	tab.iconWidget.onMouseOver:Connect(function () if self.current ~= num then tab.iconWidget:SetColor(hoverColor) end end)
	tab.iconWidget.onMouseOut:Connect(function () if self.current ~= num then tab.iconWidget:SetColor(normalColor) end end)
	tab.iconWidget.onClick:Connect(function () self:SwitchTo(tab.id) end)

	self.header:Remove(self.titleArea)
	self.header:PackEnd({ tab.iconWidget, self.titleArea })

	tab.Refresh = function ()
		if self.current ~= num then return end
		self:Refresh()
	end

	tab.SetTitle = function (t, text)
		self.title:SetText(text)
	end

	if not self.current then
		self:SwitchToNum(num)
	end

	return tab
end

function TabView.RemoveTab (self, id)
	local num = _FindTabNum(self, id)
	if not num then return end

	local tab = self.tabs[num]

	self.header:Remove(tab.iconImage)

	table.remove(self.tabs, num)

	if self.current == num then
		self:SwitchFirst()
	end
end

function TabView.SwitchToNum (self, num)
	local tab = self.tabs[num]

	if self.current then
		self.tabs[self.current].iconWidget:SetColor(normalColor)
	end

	tab.iconWidget:SetColor(activeColor)

	self.current = num

	self.title:SetText(tab.title)

	self.body:SetInnerWidget(tab.template(tab, self))
end

function TabView.SwitchTo (self, id)
	local num = _FindTabNum(self, id)
	if not num then return end

	self:SwitchToNum(num)
end

function TabView.SwitchFirst (self)
	self:SwitchToNum(1)
end

function TabView.SwitchNext (self)
	if not self.current then
		self:SwitchFirst()
	else
		local nextNum = self.current + 1
		if nextNum > #self.tabs then nextNum = 1 end
		self:SwitchToNum(nextNum)
	end
end

function TabView.SwitchPrev (self)
	if not self.current then
		self:SwitchFirst()
	else
		local nextNum = self.current - 1
		if nextNum < 1 then nextNum = #self.tabs end
		self:SwitchToNum(nextNum)
	end
end

function TabView.Refresh (self)
	if not self.current then
		self:SwitchFirst()
	else
		self:SwitchToNum(self.current)
	end
end

function TabView.SetFooter (self, footer)
	self.footer:SetInnerWidget(footer)
end

return TabView
