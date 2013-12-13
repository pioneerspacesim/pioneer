-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")

local Face = import("UI.Game.Face")

local SmallLabeledButton = import("ui/SmallLabeledButton")

local EquipmentTableWidgets = import("EquipmentTableWidgets")

local ui = Engine.ui

local l = Lang.GetResource("ui-core")

local ChatForm = {}

ChatForm.meta = {
	__index = ChatForm,
	class = "ChatForm",
}

function ChatForm.New (chatFunc, removeFunc, ref, tabGroup)
	local form = {
		chatFunc = chatFunc,
		removeFunc = removeFunc,
		ref = ref,
        tabGroup = tabGroup,
	}
	setmetatable(form, ChatForm.meta)
	form.chatFunc(form, 0)
	return form
end

function ChatForm:BuildWidget ()
	local box = ui:VBox(10)

	if self.title then
		box:PackEnd(ui:Label(self.title):SetFont("LARGE"))
	end

	if self.message or self.face then
		local hbox = ui:HBox(5)

		if self.message then
			hbox:PackEnd(ui:Expand("HORIZONTAL", ui:MultiLineText(self.message)))
		end

		if self.face then
			hbox:PackEnd(ui:Align("RIGHT", self.face))
		end

		box:PackEnd(hbox)
	end

	if self.options or self.equipWidgetConfig then

		local scrollBox = ui:VBox()

		if self.options then
			local optionBox = ui:VBox()
			for i = 1,#self.options do
				local option = self.options[i]
				local b = SmallLabeledButton.New(option[1])
				optionBox:PackEnd(b)
				b.button.onClick:Connect(function ()
					if (option[2] == -1) then
						self:Close()
					else
						self.chatFunc(self, option[2])
						ui.layer:SetInnerWidget(self:BuildWidget())
					end
				end)
			end
			scrollBox:PackEnd(optionBox)
		end

		if self.equipWidgetConfig then
			local stationTable, shipTable = EquipmentTableWidgets.Pair(self.equipWidgetConfig)

			scrollBox:PackEnd(
				ui:HBox(10):PackEnd({
					ui:VBox():PackEnd({
						ui:Label("Available for purchase"):SetFont("HEADING_LARGE"),
						stationTable,
					}),
					ui:VBox():PackEnd({
						ui:Label("In cargo hold"):SetFont("HEADING_LARGE"),
						shipTable,
					})
				})
			)
		end

		box:PackEnd(ui:Scroller(scrollBox))

	end

	local hangupButton = SmallLabeledButton.New(l.HANG_UP)
	hangupButton.button.onClick:Connect(function () self:Close() end)

	return
		ui:ColorBackground(0,0,0,0.5,
			ui:Grid({1,2,1},{1,2,1}):SetCell(1,1,
				ui:Background(
					ui:VBox(10)
						:PackEnd({ box, hangupButton })
				)
			)
		)
end

function ChatForm:SetTitle (title)
	self.title = title
end

function ChatForm:SetFace (character)

	local faceFlags = {
		character.female and "FEMALE" or "MALE",
		character.armor and "ARMOUR",
	}

	self.face = Face.New(ui, faceFlags, character.seed):SetHeightLines(5)
end

function ChatForm:SetMessage (message)
	self.message = message
end

function ChatForm:AddOption (text, option)
	self.options = self.options or {}
	table.insert(self.options, { text, option })
end

function ChatForm:Clear ()
	self.title = nil
	self.message = nil
	self.options = nil
	self.tradeFuncs = nil
end

local tradeFuncKeys = { "canTrade", "getStock", "getPrice", "onClickBuy", "onClickSell", "bought", "sold" }
function ChatForm:AddGoodsTrader (funcs)
	self.equipWidgetConfig = {
		stationColumns = { "icon", "name", "price", "stock" },
		shipColumns = { "icon", "name", "amount" },
	}

	for i = 1,#tradeFuncKeys do
		local key = tradeFuncKeys[i]
		local fn = funcs[key]
		if fn then
			self.equipWidgetConfig[key] = function (e)
				return fn(self.ref, e)
			end
		end
	end
end

function ChatForm:Close ()
	ui:DropLayer();
	if self.removeOnClose and self.removeFunc then
		self.removeFunc()
	end
end

function ChatForm:GotoPolice ()
    self:Close()
	self.tabGroup:SwitchTo("police")
end

function ChatForm:RemoveAdvertOnClose()
	self.removeOnClose = true
end

return ChatForm
