-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local StationView = import 'pigui/views/station-view'

local Game = import 'Game'
local Rand = import 'Rand'
local Format = import 'Format'
local Equipment = import 'Equipment'
local ShipDef = import 'ShipDef'
local Character = import 'Character'
local Comms = import 'Comms'

local InfoFace = import 'ui/PiguiFace'
local PiImage = import 'ui/PiImage'
local textTable = import 'pigui/libs/text-table.lua'
local ModalWindow = import 'pigui/libs/modal-win.lua'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors
local icons = ui.theme.icons

local Lang = import 'Lang'
local l = Lang.GetResource("ui-core")

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)
local widgetSizes = ui.rescaleUI({
	itemSpacing = Vector2(4, 9),
	windowPadding = Vector2(14, 14),
	faceSize = Vector2(586,565),
	buttonSizeBase = Vector2(64, 48),
}, Vector2(1600, 900))

widgetSizes.itemSpacing = Vector2(math.ceil(widgetSizes.itemSpacing.x), math.ceil(widgetSizes.itemSpacing.y))
widgetSizes.windowPadding = Vector2(math.ceil(widgetSizes.windowPadding.x), math.ceil(widgetSizes.windowPadding.y))
widgetSizes.buttonFullRefuelSize = Vector2(widgetSizes.buttonSizeBase.x*2 + widgetSizes.itemSpacing.x, widgetSizes.buttonSizeBase.y)
widgetSizes.buttonLaunchSize = Vector2(widgetSizes.buttonSizeBase.x*5, widgetSizes.buttonSizeBase.y)
widgetSizes.iconSize = Vector2(0, widgetSizes.buttonSizeBase.y)

local face = nil
local stationSeed = 0
local shipDef
local winPos = Vector2(0,0)

local hyperdrive
local hyperdrive_fuel
local hydrogenIcon = PiImage.New("icons/goods/Hydrogen.png")
local hyperdriveIcon = PiImage.New("icons/goods/Hydrogen.png")

local popupMsg = ''
local popup = ModalWindow.New('lobbyPopup', function(self)
	ui.text(popupMsg)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button("OK", Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

local requestLaunch = function ()
	local crimes, fine = Game.player:GetCrimeOutstanding()
	local station = Game.player:GetDockedWith()

	if not Game.player:HasCorrectCrew() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_CREW, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_CREW
		popup:open()
	elseif fine > 0 then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_FINED, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_FINED
		popup:open()
	elseif not Game.player:Undock() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_BUSY, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_BUSY
		popup:open()
	else
		Game.SwitchView()
	end
end

local refuelInternalTank = function (delta)
	local station = Game.player:GetDockedWith()
	local stock = station:GetEquipmentStock(Equipment.cargo.hydrogen)
	local price = station:GetEquipmentPrice(Equipment.cargo.hydrogen)

	if delta > 0 then
		if stock == 0 then
			popupMsg = l.ITEM_IS_OUT_OF_STOCK
			ui.openPopup(popupId)
			return
		end

		delta = math.clamp(delta, 0, 100 - Game.player.fuel)
	else
		delta = math.clamp(delta, -Game.player.fuel, 0)
	end

	local fuel = Game.player.fuel + delta
	local mass = shipDef.fuelTankMass/100 * delta
	local total = price * mass

	if total > Game.player:GetMoney() then
		total = Game.player:GetMoney()
		mass = total / price
		fuel = Game.player.fuel + mass * 100 / shipDef.fuelTankMass
	end

	if stock < mass then
		mass = stock
		total = price * mass
		fuel = Game.player.fuel + mass * 100 / shipDef.fuelTankMass
	end

	Game.player:AddMoney(-total)
	station:AddEquipmentStock(Equipment.cargo.hydrogen, -math.ceil(mass))
	Game.player:SetFuelPercent(fuel)
end

local refuelHyperdrive = function (mass)
	local station = Game.player:GetDockedWith()
	local stock = station:GetEquipmentStock(hyperdrive_fuel)
	local price = station:GetEquipmentPrice(hyperdrive_fuel)

	if mass > 0 then
		if stock == 0 then
			popupMsg = l.ITEM_IS_OUT_OF_STOCK
			ui.openPopup(popupId)
			return
		end

		mass = math.clamp(mass, 0, Game.player.totalCargo - Game.player.usedCargo)
	else
		mass = math.clamp(mass, -Game.player:CountEquip(hyperdrive_fuel), 0)
	end

	local total = price * mass
	if total > Game.player:GetMoney() then
		mass = math.floor(Game.player:GetMoney() / price)
		total = price * mass
	end

	if stock < mass then
		mass = stock
		total = price * mass
	end

	Game.player:AddMoney(-total)
	if mass < 0 then
		Game.player:RemoveEquip(hyperdrive_fuel, math.abs(mass), "cargo")
	else
		Game.player:AddEquip(hyperdrive_fuel, mass, "cargo")
	end
	station:AddEquipmentStock(hyperdrive_fuel, -mass)
end

local function lobbyMenu(startPos)
	local station = Game.player:GetDockedWith()
	ui.setCursorPos(startPos)
	ui.columns(4, 'thrusterFuel', false)
	ui.setColumnWidth(0, widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)
	ui.setColumnWidth(1, widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)
	ui.setColumnWidth(2, (widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)*4)
	-- internal tank fuel
	hydrogenIcon:Draw(widgetSizes.iconSize)
	ui.nextColumn()
	ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
		ui.text(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen)) .. "/t")
		ui.text(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen) * shipDef.fuelTankMass/100 * Game.player.fuel))
	end)
	ui.nextColumn()
	if ui.coloredSelectedButton(l.REFUEL_FULL, widgetSizes.buttonFullRefuelSize, false, colors.buttonBlue, nil, true) then refuelInternalTank(100) end
	ui.sameLine()
	if ui.coloredSelectedButton("-10%", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelInternalTank(-10) end
	ui.sameLine()
	if ui.coloredSelectedButton("+10%", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelInternalTank(10) end
	ui.nextColumn()
	local gaugePos = ui.getCursorScreenPos()
	gaugePos.y = gaugePos.y + widgetSizes.buttonSizeBase.y/2
	local gaugeWidth = ui.getContentRegion().x - widgetSizes.itemSpacing.x
	ui.gauge(gaugePos, Game.player.fuel, '', string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
			shipDef.fuelTankMass/100 * Game.player.fuel, Game.player:GetRemainingDeltaV()/1000),
			0, 100, icons.fuel,
			colors.gaugeEquipmentMarket, '', gaugeWidth, widgetSizes.buttonSizeBase.y, ui.fonts.pionillium.medlarge)
	-- hyperspace fuel
	ui.nextColumn()
	hyperdriveIcon:Draw(widgetSizes.iconSize)
	ui.nextColumn()
	ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
		ui.text(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel)) .. "/t")
		ui.text(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel) * Game.player:CountEquip(hyperdrive_fuel)))
	end)
	ui.nextColumn()
	if ui.coloredSelectedButton("-10t", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(-10) end
	ui.sameLine()
	if ui.coloredSelectedButton("-1t", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(-1) end
	ui.sameLine()
	if ui.coloredSelectedButton("+1t", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(1) end
	ui.sameLine()
	if ui.coloredSelectedButton("+10t", widgetSizes.buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(10) end
	ui.nextColumn()
	gaugePos = ui.getCursorScreenPos()
	gaugePos.y = gaugePos.y + widgetSizes.buttonSizeBase.y/2
	ui.gauge(gaugePos, Game.player:CountEquip(hyperdrive_fuel), '', string.format(l.FUEL .. ": %dt \t" .. l.HYPERSPACE_RANGE .. ": %d " .. l.LY,
			Game.player:CountEquip(hyperdrive_fuel), Game.player:GetHyperspaceRange()),
			0, Game.player.totalCargo - Game.player.usedCargo + Game.player:CountEquip(hyperdrive_fuel),
			icons.hyperspace, colors.gaugeEquipmentMarket, '',
			gaugeWidth, widgetSizes.buttonSizeBase.y, ui.fonts.pionillium.medlarge)

	ui.columns(1, '', false)
	ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
		local buttonPos = ui.getCursorScreenPos()
		buttonPos.x = gaugePos.x + gaugeWidth - widgetSizes.buttonLaunchSize.x
		ui.setCursorScreenPos(buttonPos)
		if ui.coloredSelectedButton(l.REQUEST_LAUNCH, widgetSizes.buttonLaunchSize, false, colors.buttonBlue, nil, true) then requestLaunch() end
	end)
end

local function drawPlayerInfo()
	local station = Game.player:GetDockedWith()

	if(not (station and shipDef)) then return end

	local tech_certified

	if station.techLevel == 11 then
		tech_certified = l.TECH_CERTIFIED_MILITARY
	else
		tech_certified = string.interp(l.TECH_CERTIFIED, { tech_level = station.techLevel})
	end

	local station_docks = string.interp(l.STATION_DOCKS, { total_docking_pads = string.format("%d", station.numDocks),})

	local orbit_period = station.path:GetSystemBody().orbitPeriod

	local station_orbit_info = ""
	if station.type == "STARPORT_ORBITAL" then
		station_orbit_info =
		string.interp(l.STATION_ORBIT, { orbit_period = string.format("%.2f", orbit_period),
										 parent_body = station.path:GetSystemBody().parent.name})
	end

	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.withStyleVars({WindowPadding = widgetSizes.windowPadding, ItemSpacing = widgetSizes.itemSpacing}, function()
			local conReg = ui.getContentRegion()
			local infoColumnWidth = conReg.x - widgetSizes.faceSize.x - widgetSizes.windowPadding.x*3
			local lobbyMenuHeight = widgetSizes.buttonSizeBase.y*3 + widgetSizes.itemSpacing.y*2
			local lobbyMenuAtBottom = (conReg.y - widgetSizes.faceSize.y > lobbyMenuHeight + widgetSizes.windowPadding.x*2)

			ui.child("Wrapper", Vector2(0, widgetSizes.faceSize.y + widgetSizes.windowPadding.y*2 + widgetSizes.itemSpacing.y), {}, function()
				ui.child("PlayerShipFuel", Vector2(infoColumnWidth, 0), {"AlwaysUseWindowPadding"}, function()
					local curPos = ui.getCursorPos()
					textTable.withHeading(station.label, orbiteer.xlarge, {
						{ tech_certified, "" },
						{ station_docks, "" },
						{ station_orbit_info, "" },
					})

					if not lobbyMenuAtBottom then
						lobbyMenu(Vector2(curPos.x, curPos.y + widgetSizes.faceSize.y - lobbyMenuHeight))
					end
				end)

				ui.sameLine()
				ui.child("StationManager", Vector2(0, 0), {"AlwaysUseWindowPadding", "NoScrollbar"}, function ()
					if(face ~= nil) then
						face:render()
					end
				end)
			end)

			if lobbyMenuAtBottom then
				ui.child("LobbyMenuPanel", Vector2(0,0), {"AlwaysUseWindowPadding"}, function()
					lobbyMenu(Vector2(0,0))
				end)
			end

		end)
	end)
end

StationView:registerView({
	id = "lobby",
	name = l.LOBBY,
	icon = ui.theme.icons.info,
	showView = true,
	draw = function()
		widgetSizes.info_column_width = ui.getContentRegion().x - widgetSizes.faceSize.x - widgetSizes.windowPadding.x
		ui.child("StationLobby", Vector2(0, ui.getContentRegion().y - StationView.style.height), {}, drawPlayerInfo)

		StationView:shipSummary()
	end,
	refresh = function()
		local station = Game.player:GetDockedWith()
		shipDef = ShipDef[Game.player.shipId]
		if (station) then
			if (stationSeed ~= station.seed) then
				stationSeed = station.seed
				local rand = Rand.New(station.seed)
				face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand), {windowPadding = widgetSizes.windowPadding, itemSpacing = widgetSizes.itemSpacing, size = widgetSizes.faceSize})
			end
			hyperdrive = table.unpack(Game.player:GetEquip("engine")) or nil
			hyperdrive_fuel = hyperdrive and hyperdrive.fuel or Equipment.cargo.hydrogen
			hyperdriveIcon = PiImage.New("icons/goods/" .. hyperdrive_fuel.icon_name .. ".png")
		end
	end,
})
