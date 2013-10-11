-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")

local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui

local ChatForm = {}

ChatForm.meta = {
	__index = ChatForm,
	class = "ChatForm",
}

function ChatForm.New ()
	local form = {}
	setmetatable(form, ChatForm.meta)
	return form
end

function ChatForm:BuildWidget ()
	local box = ui:VBox(10)

	if self.title then
		box:PackEnd(ui:Label(self.title):SetFont("LARGE"))
	end

	if self.message then
		box:PackEnd(ui:MultiLineText(self.message))
	end

	if self.options then
		local optionBox = ui:VBox()
		for i = 1,#self.options do
			local option = self.options[i]
			local b = SmallLabeledButton.New(option[1])
			optionBox:PackEnd(b)
			b.button.onClick:Connect(function ()
				print(string.format("clicked %d", option[2]))
				if (option[2] == -1) then
					ui:DropLayer()
				end
			end)
		end
		box:PackEnd(optionBox)
	end

	return box
end

function ChatForm:SetTitle (title)
	print(title)
	self.title = title
end

function ChatForm:SetFace (props)
    print("ChatForm:SetFace")
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
    print("ChatForm:RemoveAdvertOnClose")
end

return ChatForm
