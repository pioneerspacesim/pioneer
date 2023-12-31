-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'

local CommodityWidget = require 'pigui.libs.commodity-market'
local Face = require 'pigui.libs.face'
local Vector2 = _G.Vector2
local Color = _G.Color

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local l = Lang.GetResource("ui-core")

local ChatForm = {}

ChatForm.meta = {
	__index = ChatForm,
	class = "ChatForm",
}

function ChatForm.New (chatFunc, removeFunc, closeFunc, resizeFunc, ref, tabGroup, style)
	style = style or {}
	style.face = style.face or {}
	local form = {
		chatFunc = chatFunc,
		removeFunc = removeFunc,
		closeFunc = closeFunc,
		resizeFunc = resizeFunc,
		ref = ref,
		tabGroup = tabGroup,
		market = nil,
		navButton = function() end,
		style = {
			face = {
				size = style.face.size or ui.rescaleUI(Vector2(96, 96), Vector2(1600, 900)),
				windowPadding = style.face.windowPadding or ui.rescaleUI(Vector2(4, 4), Vector2(1600, 900)),
				itemSpacing = style.face.itemSpacing or ui.rescaleUI(Vector2(4, 4), Vector2(1600, 900)),
				showCharInfo = style.face.showCharInfo or false,
				charInfoPadding = style.face.charInfoPadding or ui.rescaleUI(Vector2(4,4), Vector2(1600, 900)),
				charInfoBgColor = style.face.bgColor or Color(0,0,0,160),
				nameFont = style.face.nameFont or orbiteer.small,
				titleFont = style.face.titleFont or orbiteer.tiny,
			},
			buttonSize = style.buttonSize or ui.rescaleUI(Vector2(256, 24), Vector2(1600, 900)),
			titleFont = style.titleFont or orbiteer.large,
			font = style.font or pionillium.medlarge,
			marketSize = style.marketSize or ui.rescaleUI(Vector2(1200, 450), Vector2(1600, 900))
		},
	}
	-- So that the font at least fits in the button in height
	form.style.buttonSize = Vector2(form.style.buttonSize.x, math.max(form.style.buttonSize.y, form.style.font.size + ui.theme.styles.ButtonPadding.y * 2))
	setmetatable(form, ChatForm.meta)
	form.chatFunc(form, 0)
	return form
end

function ChatForm:render ()
	ui.withFont(self.style.font.name, self.style.font.size, function()
		if not self.style.contentWidth then
			self.style.contentWidth = ui.getContentRegion().x
		end

		if self.title then
			ui.withFont(self.style.titleFont.name, self.style.titleFont.size, function()
				ui.pushTextWrapPos(self.style.contentWidth)
				ui.textWrapped(self.title)
				ui.popTextWrapPos()
			end)
		end

		if self.face then
			self.face:render()
		end

		if self.message then
			ui.text('')
			ui.pushTextWrapPos(self.style.contentWidth)
			ui.textWrapped(self.message)
			ui.popTextWrapPos()
			ui.text('')
		end

		if self.style.buttonSize.x <= 0 then
			self.style.buttonSize.x = self.style.contentWidth
		end

		if self.options then
			for _, option in ipairs(self.options) do
				if ui.button(option[1], self.style.buttonSize) then
					if (option[2] == -1) then
						self:Close()
					else
						self.chatFunc(self, option[2])
					end
				end
			end
		end

		if self.market then
			self.market:Render(self.style.marketSize)
		end

		self.navButton()

		if ui.button(l.HANG_UP, self.style.buttonSize) or ui.escapeKeyReleased(true) then
			self:Close()
		end
	end)
end

function ChatForm:SetTitle (title)
	self.title = title
end

function ChatForm:SetFace (character)
	self.face = Face.New(character, self.style.face)
end

function ChatForm:ClearFace()
	self.face = nil
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
	self.market = nil
end

local tradeFuncKeys = { "canTrade", "getStock", "getBuyPrice", "getSellPrice", "onClickBuy", "onClickSell", "canDisplayItem", "bought", "sold"}
function ChatForm:AddGoodsTrader (funcs)
	self.equipWidgetConfig = {
		stationColumns = { "icon", "name", "price", "stock" },
		shipColumns = { "icon", "name", "amount" },
	}

	self.market = CommodityWidget.New("chatTrader", false)
	for i = 1,#tradeFuncKeys do
		local key = tradeFuncKeys[i]
		local fn = funcs[key]
		if fn then
			self.market.funcs[key] = function (s, e)
				return fn(self.ref, e, s)
			end
		end
	end

	self.resizeFunc()
	self.market:Refresh()
end

function ChatForm:AddNavButton (target)
	self.navButton = function()
		if ui.button(l.SET_AS_TARGET, self.style.buttonSize) then
			if target:isa("Body") and target:IsDynamic() then
				Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			elseif Game.system and target:IsSameSystem(Game.system.path) then
				if target.bodyIndex then
					Game.player:SetNavTarget(Space.GetBody(target.bodyIndex))
					ui.playSfx("OK")
				end
			elseif not Game.InHyperspace() then
				-- if a specific systembody is given, set the sector map to the correct star (if the system is multiple)
				Game.sectorView:SwitchToPath(target:IsBodyPath() and target:GetSystemBody().nearestJumpable.path or target:GetStarSystem().path)
				ui.playBoinkNoise()
			end
		end
	end
end

function ChatForm:RemoveNavButton ()
	self.navButton = function() end
end

function ChatForm:Close ()
	self.closeFunc()
end

function ChatForm:GotoPolice ()
	self:Close()
	self.tabGroup:SwitchTo("police")
end

function ChatForm:RemoveAdvertOnClose()
	-- removing it now. has the same effect for the player, without requiring
	-- us to somehow hook the entire view being closed (eg switch back to worldview)
	if self.removeFunc then
		self.removeFunc()
	end
end

return ChatForm
