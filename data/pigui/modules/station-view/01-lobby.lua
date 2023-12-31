-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local StationView = require 'pigui.views.station-view'
local AutoSave    = require 'modules.AutoSave.AutoSave'

local Game = require 'Game'
local Rand = require 'Rand'
local Format = require 'Format'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Character = require 'Character'
local Commodities = require 'Commodities'
local Comms = require 'Comms'
local Space = require 'Space'

local PiGuiFace = require 'pigui.libs.face'
local PiImage = require 'pigui.libs.image'
local textTable = require 'pigui.libs.text-table'
local ModalWindow = require 'pigui.libs.modal-win'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors
local icons = ui.theme.icons

local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")
local Vector2 = _G.Vector2

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)
local widgetSizes = ui.rescaleUI({
	itemSpacing = Vector2(4, 9),
	faceSize = Vector2(586,565),
	buttonSizeBase = Vector2(72, 48),
}, Vector2(1600, 900))

widgetSizes.itemSpacing = Vector2(math.ceil(widgetSizes.itemSpacing.x), math.ceil(widgetSizes.itemSpacing.y))
widgetSizes.buttonFullRefuelSize = Vector2(widgetSizes.buttonSizeBase.x*2 + widgetSizes.itemSpacing.x, widgetSizes.buttonSizeBase.y)
widgetSizes.buttonLaunchSize = Vector2(widgetSizes.buttonSizeBase.x*5, widgetSizes.buttonSizeBase.y)
widgetSizes.iconSize = Vector2(0, widgetSizes.buttonSizeBase.y)

local face = nil
local stationSeed = 0
local shipDef

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

local requestLaunch = function (station)
	local _, fine = Game.player:GetCrimeOutstanding()
	local nearbyTraffic = station:GetNearbyTraffic(50000) -- ships within 50km of station

	if not Game.player:HasCorrectCrew() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_CREW, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_CREW
		popup:open()
		return
	elseif fine > 0 then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_FINED, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_FINED
		popup:open()
		return
	end

	AutoSave.Save()

	if not Game.player:Undock() then
		Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_BUSY, station.label)
		popupMsg = l.LAUNCH_PERMISSION_DENIED_BUSY
		popup:open()
	elseif nearbyTraffic - (station.numShipsDocked) > 0 then -- station radar picking up stuff nearby
		Comms.Message(l.LAUNCH_PERMISSION_GRANTED_WATCH_TRAFFIC, station.label)
		Game.SwitchView()
	elseif station.numDocks - (station.numShipsDocked + 1) < station.numShipsDocked * 0.2 then -- busy station, pads almost full
		Comms.Message(l.LAUNCH_PERMISSION_GRANTED_DEPART_QUICK, station.label)
		Game.SwitchView()
	else
		Comms.Message(l.LAUNCH_PERMISSION_GRANTED, station.label)
		Game.SwitchView()
	end
end

local refuelInternalTank = function (delta)
	local station = Game.player:GetDockedWith()
	local stock = station:GetCommodityStock(Commodities.hydrogen)
	local price = station:GetCommodityPrice(Commodities.hydrogen)

	if delta > 0 then
		if stock == 0 then
			popupMsg = l.ITEM_IS_OUT_OF_STOCK
			popup:open()
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
	local commodityChangeAmount = mass < 0 and math.floor(mass) or math.ceil(mass)
	station:AddCommodityStock(Commodities.hydrogen, -commodityChangeAmount)
	Game.player:SetFuelPercent(fuel)
end

local refuelHyperdrive = function (mass)
	local station = Game.player:GetDockedWith()
	local stock = station:GetCommodityStock(hyperdrive_fuel)
	local price = station:GetCommodityPrice(hyperdrive_fuel)

	---@type CargoManager
	local cargoMgr = Game.player:GetComponent('CargoManager')

	if mass > 0 then
		if stock == 0 then
			popupMsg = l.ITEM_IS_OUT_OF_STOCK
			popup:open()
			return
		end

		mass = math.clamp(mass, 0, cargoMgr:GetFreeSpace())
	else
		mass = math.clamp(mass, -cargoMgr:CountCommodity(hyperdrive_fuel), 0)
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
		cargoMgr:RemoveCommodity(hyperdrive_fuel, math.abs(mass))
	else
		cargoMgr:AddCommodity(hyperdrive_fuel, mass)
	end
	station:AddCommodityStock(hyperdrive_fuel, -mass)
end

local function lobbyMenu()
	local station = Game.player:GetDockedWith()
	if not station then return end -- station can be false if we requested launch this frame

	-- Setup columns: icon | costs | buttons | fuel gauge
	ui.columns(4, 'thrusterFuel', false)
	ui.setColumnWidth(0, widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)
	ui.setColumnWidth(1, widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)
	ui.setColumnWidth(2, (widgetSizes.buttonSizeBase.x + widgetSizes.itemSpacing.x)*4)

	-- internal tank fuel
	hydrogenIcon:Draw(widgetSizes.iconSize)
	ui.nextColumn()

	-- internal tank refill costs
	ui.withFont(pionillium.body, function()
		ui.text(Format.Money(station:GetCommodityPrice(Commodities.hydrogen)) .. "/t")
		ui.text(Format.Money(station:GetCommodityPrice(Commodities.hydrogen) * shipDef.fuelTankMass/100 * Game.player.fuel))
	end)
	ui.nextColumn()

	-- internal tank refill buttons
	if ui.button(l.REFUEL_FULL, widgetSizes.buttonFullRefuelSize) then refuelInternalTank(100) end
	ui.sameLine()
	if ui.button("-10%", widgetSizes.buttonSizeBase) then refuelInternalTank(-10) end
	ui.sameLine()
	if ui.button("+10%", widgetSizes.buttonSizeBase) then refuelInternalTank(10) end
	ui.nextColumn()

	-- internal tank fuel gauge
	local gaugePos = ui.getCursorScreenPos()
	local gaugeHeight = widgetSizes.buttonSizeBase.y
	gaugePos.y = gaugePos.y + widgetSizes.buttonSizeBase.y/2
	local gaugeWidth = ui.getContentRegion().x
	ui.gauge(gaugePos, Game.player.fuel, '', string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
		shipDef.fuelTankMass/100 * Game.player.fuel, Game.player:GetRemainingDeltaV()/1000),
		0, 100, icons.fuel,
		colors.gaugeEquipmentMarket, '', gaugeWidth, gaugeHeight, pionillium.body)

	-- hyperspace fuel
	ui.nextColumn()

	hyperdriveIcon:Draw(widgetSizes.iconSize)
	ui.nextColumn()

	---@type CargoManager
	local cargoMgr = Game.player:GetComponent('CargoManager')
	local stored_hyperfuel = cargoMgr:CountCommodity(hyperdrive_fuel)

	-- hyperspace fuel prices
	ui.withFont(pionillium.body, function()
		ui.text(Format.Money(station:GetCommodityPrice(hyperdrive_fuel)) .. "/t")
		ui.text(Format.Money(station:GetCommodityPrice(hyperdrive_fuel) * stored_hyperfuel))
	end)
	ui.nextColumn()

	-- hyperspace fuel refill buttons
	if ui.button("-10t", widgetSizes.buttonSizeBase) then refuelHyperdrive(-10) end
	ui.sameLine()
	if ui.button("-1t", widgetSizes.buttonSizeBase) then refuelHyperdrive(-1) end
	ui.sameLine()
	if ui.button("+1t", widgetSizes.buttonSizeBase) then refuelHyperdrive(1) end
	ui.sameLine()
	if ui.button("+10t", widgetSizes.buttonSizeBase) then refuelHyperdrive(10) end
	ui.nextColumn()

	-- hyperspace fuel gauge
	gaugePos = ui.getCursorScreenPos()
	gaugePos.y = gaugePos.y + widgetSizes.buttonSizeBase.y/2
	ui.gauge(gaugePos, stored_hyperfuel, '', string.format(l.FUEL .. ": %dt \t" .. l.HYPERSPACE_RANGE .. ": %d " .. l.LY,
		stored_hyperfuel, Game.player:GetHyperspaceRange()),
		0, cargoMgr:GetFreeSpace() + stored_hyperfuel,
		icons.hyperspace, colors.gaugeEquipmentMarket, '',
		gaugeWidth, gaugeHeight, pionillium.body)

	ui.columns(1, '', false)
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

	local station_frameBody = Space.GetBody(station.path:GetSystemBody().parent.index)
	local local_gravity_pressure = ""
	if station.type == "STARPORT_SURFACE" then
		if station.path:GetSystemBody().parent.hasAtmosphere then
			local_gravity_pressure = string.format(l.STATION_LOCAL_GRAVITY_PRESSURE, (station.path:GetSystemBody().parent.gravity/9.8), station_frameBody:GetAtmosphericState(station))
		else
			local_gravity_pressure = string.format(l.STATION_LOCAL_GRAVITY, (station.path:GetSystemBody().parent.gravity/9.8))
		end
	end

	local station_orbit_info = ""
	if station.type == "STARPORT_ORBITAL" then
		station_orbit_info =
			string.interp(l.STATION_ORBIT, { orbit_period = string.format("%.2f", orbit_period),
				parent_body = station.path:GetSystemBody().parent.name})
	end

	ui.withFont(pionillium.heading, function()
		ui.withStyleVars({ItemSpacing = widgetSizes.itemSpacing}, function()
			local buttonSizeSpacing = widgetSizes.buttonLaunchSize.y + widgetSizes.itemSpacing.y
			local lobbyMenuHeight = widgetSizes.buttonSizeBase.y*2 + widgetSizes.itemSpacing.y*3 -- use an extra itemSpacing to avoid scrollbar

			ui.child("Wrapper", Vector2(0, -lobbyMenuHeight), {}, function()
				-- face display has 1:1 aspect ratio, and we need size for a launch button underneath
				local infoColumnWidth = -math.min(ui.getContentRegion().y - buttonSizeSpacing, widgetSizes.faceSize.x) - widgetSizes.itemSpacing.x
				ui.child("PlayerShipFuel", Vector2(infoColumnWidth, 0), function()
					textTable.withHeading(station.label, orbiteer.title, {
						{ tech_certified, "" },
						{ station_docks, "" },
						{ station_orbit_info, "" },
						{ local_gravity_pressure, ""},
					})
				end)

				ui.sameLine()

				ui.group(function()
					if(face ~= nil) then face:render() end

					ui.withFont(orbiteer.title, function()
						local size = Vector2(ui.getContentRegion().x, widgetSizes.buttonLaunchSize.y)
						if ui.button(l.REQUEST_LAUNCH, size) then
							requestLaunch(station)
						end
					end)
				end)
			end)

			ui.child("LobbyMenuPanel", Vector2(0,0), lobbyMenu)

		end)
	end)
end

StationView:registerView({
	id = "lobby",
	name = l.LOBBY,
	icon = ui.theme.icons.info,
	showView = true,
	draw = drawPlayerInfo,
	refresh = function()
		local station = Game.player:GetDockedWith()
		shipDef = ShipDef[Game.player.shipId]

		if (station) then
			if (stationSeed ~= station.seed) then
				stationSeed = station.seed
				local rand = Rand.New(station.seed)
				face = PiGuiFace.New(Character.New({ title = l.STATION_MANAGER }, rand), {itemSpacing = widgetSizes.itemSpacing})
			end

			hyperdrive = table.unpack(Game.player:GetEquip("engine")) or nil
			hyperdrive_fuel = hyperdrive and hyperdrive.fuel or Commodities.hydrogen
			hyperdriveIcon = PiImage.New("icons/goods/" .. hyperdrive_fuel.icon_name .. ".png")
		end
	end,
	debugReload = function()
		package.reimport()
	end
})
