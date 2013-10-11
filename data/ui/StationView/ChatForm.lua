-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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

function ChatForm:SetTitle (title)
    print(string.format("ChatForm:SetTitle: %s", title))
end

function ChatForm:SetFace (props)
    print("ChatForm:SetFace")
end

function ChatForm:SetMessage (message)
    print(string.format("ChatForm:Message: %s", message))
end

function ChatForm:AddOption (text, option)
    print(string.format("ChatForm:AddOption: %d %s", option, text))
end

function ChatForm:Clear ()
    print("ChatForm:Clear")
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
