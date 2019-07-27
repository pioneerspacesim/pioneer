-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
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
local drawTable = import 'pigui/libs/table.lua'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors
local icons = ui.theme.icons

local Lang = import 'Lang'
local l = Lang.GetResource("ui-core")

local itemSpacing = Vector2(math.ceil(6 * (ui.screenHeight / 1200)), math.ceil(12 * (ui.screenHeight / 1200)))
local windowPadding = Vector2(math.ceil(18 * (ui.screenHeight / 1200)), math.ceil(18 * (ui.screenHeight / 1200)))
local faceSize = Vector2(440,424) * ui.screenWidth / 1200
local info_column_width = ui.screenWidth - faceSize.x
local buttonSizeBase = Vector2(math.ceil(85 * (ui.screenHeight / 1200)), math.ceil(64 * (ui.screenHeight / 1200)))
local buttonFullRefuelSize = Vector2(buttonSizeBase.x*2 + itemSpacing.x, buttonSizeBase.y)
local buttonLaunchSize = Vector2(buttonSizeBase.x*5, buttonSizeBase.y)
local iconSize = Vector2(0, buttonSizeBase.y)

local face;
local shipDef
local winPos = Vector2(0,0)

local hyperdrive
local hyperdrive_fuel
local hydrogenIcon = PiImage.New("icons/goods/Hydrogen.png")
local hyperdriveIcon = PiImage.New("icons/goods/Hydrogen.png")

local popupMsg = ''
local popupId = 'lobbyPopup'

local requestLaunch = function ()
	local crimes, fine = Game.player:GetCrimeOutstanding()
	local station = Game.player:GetDockedWith()

	if not Game.player:HasCorrectCrew() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_CREW, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_CREW
		ui.openPopup(popupId)
	elseif fine > 0 then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_FINED, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_FINED
		ui.openPopup(popupId)
	elseif not Game.player:Undock() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_BUSY, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_BUSY
		ui.openPopup(popupId)
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

local function drawPlayerInfo()
	local station = Game.player:GetDockedWith()

	if(not (station and shipDef)) then return end

	local tech_certified

	if station.techLevel == 11 then
		tech_certified = l.TECH_CERTIFIED_MILITARY
	else
		tech_certified = string.interp(l.TECH_CERTIFIED, { tech_level = station.techLevel})
	end

	local orbit_period = station.path:GetSystemBody().orbitPeriod

	local station_orbit_info = ""
	if station.type == "STARPORT_ORBITAL" then
		station_orbit_info =
		string.interp(l.STATION_ORBIT, { orbit_period = string.format("%.2f", orbit_period),
										 parent_body = station.path:GetSystemBody().parent.name})
	end

	ui.withFont(pionillium.medlarge.name, pionillium.large.size, function()
		ui.withStyleVars({WindowPadding = windowPadding, ItemSpacing = itemSpacing}, function()
			ui.child("PlayerShipFuel", Vector2(info_column_width, 0), {"AlwaysUseWindowPadding"}, function()
				local curPos = ui.getCursorPos()
				drawTable.withHeading(station.label, orbiteer.xlarge, {
					{ tech_certified, "" },
					{ station_orbit_info, "" },
				})
				ui.setCursorPos(Vector2(curPos.x, curPos.y + faceSize.y - buttonSizeBase.y*3 - itemSpacing.y*2))
				ui.columns(4, 'thrusterFuel', false)
				ui.setColumnWidth(0, buttonSizeBase.x + itemSpacing.x)
				ui.setColumnWidth(1, buttonSizeBase.x + itemSpacing.x)
				ui.setColumnWidth(2, (buttonSizeBase.x + itemSpacing.x)*4)
				-- internal tank fuel
				hydrogenIcon:Draw(iconSize)
				ui.nextColumn()
				ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
					ui.text(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen)) .. "/t")
					ui.text(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen) * shipDef.fuelTankMass/100 * Game.player.fuel))
				end)
				ui.nextColumn()
				if ui.coloredSelectedButton(l.REFUEL_FULL, buttonFullRefuelSize, false, colors.buttonBlue, nil, true) then refuelInternalTank(100) end
				ui.sameLine()
				if ui.coloredSelectedButton("-10%", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelInternalTank(-10) end
				ui.sameLine()
				if ui.coloredSelectedButton("+10%", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelInternalTank(10) end
				ui.nextColumn()
				local gaugePos = ui.getWindowPos() + ui.getCursorPos()
				gaugePos.y = gaugePos.y + windowPadding.y + itemSpacing.y
				local gaugeWidth = ui.getContentRegion().x
				ui.gauge(gaugePos, Game.player.fuel, '', string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
						shipDef.fuelTankMass/100 * Game.player.fuel, Game.player:GetRemainingDeltaV()/1000),
						0, 100, icons.fuel,
						colors.gaugeEquipmentMarket, '', gaugeWidth, 48, ui.fonts.pionillium.medlarge)
				-- hyperspace fuel
				ui.nextColumn()
				hyperdriveIcon:Draw(iconSize)
				ui.nextColumn()
				ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
					ui.text(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel)) .. "/t")
					ui.text(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel) * Game.player:CountEquip(hyperdrive_fuel)))
				end)
				ui.nextColumn()
				if ui.coloredSelectedButton("-10t", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(-10) end
				ui.sameLine()
				if ui.coloredSelectedButton("-1t", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(-1) end
				ui.sameLine()
				if ui.coloredSelectedButton("+1t", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(1) end
				ui.sameLine()
				if ui.coloredSelectedButton("+10t", buttonSizeBase, false, colors.buttonBlue, nil, true) then refuelHyperdrive(10) end
				ui.nextColumn()
				gaugePos = ui.getWindowPos() + ui.getCursorPos()
				gaugePos.y = gaugePos.y + windowPadding.y + itemSpacing.y
				gaugeWidth = ui.getContentRegion().x
				ui.gauge(gaugePos, Game.player:CountEquip(hyperdrive_fuel), '', string.format(l.FUEL .. ": %dt \t" .. l.HYPERSPACE_RANGE .. ": %d " .. l.LY,
						Game.player:CountEquip(hyperdrive_fuel), Game.player:GetHyperspaceRange()),
						0, Game.player.totalCargo - Game.player.usedCargo + Game.player:CountEquip(hyperdrive_fuel),
						icons.hyperspace, colors.gaugeEquipmentMarket, '',
						gaugeWidth, 48, ui.fonts.pionillium.medlarge)
				curPos = ui.getCursorPos()
				ui.columns(1, '', false)
				ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
					ui.dummy(Vector2(info_column_width - buttonLaunchSize.x - (windowPadding.x + itemSpacing.x)*2, 5))
					ui.sameLine()
					if ui.coloredSelectedButton(l.REQUEST_LAUNCH, buttonLaunchSize, false, colors.buttonBlue, nil, true) then requestLaunch() end
				end)

				ui.setNextWindowSize(Vector2(0, 0), "Always")
				ui.popupModal(popupId, {"NoTitleBar", "NoResize"}, function ()
					ui.text(popupMsg)
					ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
					ui.sameLine()
					if ui.button("OK", Vector2(100, 0)) then
						ui.closeCurrentPopup()
					end
				end)
			end)

			ui.sameLine()
			ui.child("StationManager", Vector2(faceSize.x, 0), {"AlwaysUseWindowPadding"}, function ()
				if(face ~= nil) then
					face:Draw(faceSize)
				end
			end)
		end)
	end)
end

StationView:registerView({
	id = "lobby",
	name = l.LOBBY,
	icon = ui.theme.icons.info,
	showView = true,
	draw = function()
		winPos = ui.getWindowPos();
		info_column_width = ui.getContentRegion().x - faceSize.x - windowPadding.x
		ui.child("StationLobby", Vector2(0, ui.getContentRegion().y - 100), {}, drawPlayerInfo)

		StationView:shipSummary()
	end,
	refresh = function()
		local station = Game.player:GetDockedWith()
		shipDef = ShipDef[Game.player.shipId]
		if(station) then
			local rand = Rand.New(station.seed)
			face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand), {windowPadding = windowPadding, itemSpacing = itemSpacing})
			hyperdrive = table.unpack(Game.player:GetEquip("engine")) or nil
			hyperdrive_fuel = hyperdrive and hyperdrive.fuel or Equipment.cargo.hydrogen
			hyperdriveIcon = PiImage.New("icons/goods/" .. hyperdrive_fuel.icon_name .. ".png")
		end
	end,
})