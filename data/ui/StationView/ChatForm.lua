-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")

local Face = import("UI.Game.Face")

local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui

local l = Lang.GetResource("ui-core")

local ChatForm = {}

ChatForm.meta = {
	__index = ChatForm,
	class = "ChatForm",
}

function ChatForm.New (chatFunc, removeFunc)
	local form = {
		chatFunc = chatFunc,
		removeFunc = removeFunc,
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
			hbox:PackEnd(ui:MultiLineText(self.message))
		end

		if self.face then
			hbox:PackEnd(self.face)
		end

		box:PackEnd(hbox)
	end

	local function closeForm ()
		ui:DropLayer();
		if self.removeOnClose and self.removeFunc then
			self.removeFunc()
		end
	end

	if self.options then
		local optionBox = ui:VBox()
		for i = 1,#self.options do
			local option = self.options[i]
			local b = SmallLabeledButton.New(option[1])
			optionBox:PackEnd(b)
			b.button.onClick:Connect(function ()
				if (option[2] == -1) then
					closeForm()
				else
					self.chatFunc(self, option[2])
					ui.layer:SetInnerWidget(self:BuildWidget())
				end
			end)
		end
		box:PackEnd(optionBox)
	end

	local hangupButton = SmallLabeledButton.New(l.HANG_UP)
	hangupButton.button.onClick:Connect(closeForm)

	return
		ui:ColorBackground(0,0,0,0.5,
			ui:Align("MIDDLE",
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
end

function ChatForm:AddGoodsTrader (funcs)
	print("ChatForm:AddGoodsTrader")
end

function ChatForm:Close ()
	print("ChatForm:Close")
end

function ChatForm:GotoPolice ()
	print("ChatForm:GotoPolice")
end

function ChatForm:RemoveAdvertOnClose()
	self.removeOnClose = true
end

return ChatForm
