local ui = Engine.ui
local t = Translate:GetTranslator()

local shipInfo = function (args)
	local shipId = Game.player.shipType
	local shipType = ShipType.GetShipType(shipId)

	local hyperdrive =              table.unpack(Game.player:GetEquip("ENGINE"))
	local frontWeapon, rearWeapon = table.unpack(Game.player:GetEquip("LASER"))

	hyperdrive =  hyperdrive  or "NONE"
	frontWeapon = frontWeapon or "NONE"
	rearWeapon =  rearWeapon  or "NONE"

	local stats = Game.player:GetStats()

	local equipColumn = { {}, {} }
	local columnNum = 1
	for i = 1,#Constants.EquipType do
		local type = Constants.EquipType[i]
		local et = EquipType.GetEquipType(type)
		local slot = et.slot
		if (slot ~= "CARGO" and slot ~= "MISSILE" and slot ~= "ENGINE" and slot ~= "LASER") then
			local count = Game.player:GetEquipCount(slot, type)
			if count > 0 then
				if count > 1 then
					if type == "SHIELD_GENERATOR" then
						table.insert(equipColumn[columnNum],
							ui:Label(string.interp(t("{quantity} Shield Generators"), { quantity = string.format("%d", count) })))
					elseif type == "PASSENGER_CABIN" then
						table.insert(equipColumn[columnNum],
							ui:Label(string.interp(t("{quantity} Occupied Passenger Cabins"), { quantity = string.format("%d", count) })))
					elseif type == "UNOCCUPIED_CABIN" then
						table.insert(equipColumn[columnNum],
							ui:Label(string.interp(t("{quantity} Unoccupied Passenger Cabins"), { quantity = string.format("%d", count) })))
					else
						table.insert(equipColumn[columnNum], ui:Label(et.name))
					end
				else
					table.insert(equipColumn[columnNum], ui:Label(et.name))
				end
				columnNum = columnNum == 1 and 2 or 1
			end
		end
	end

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Label(t("Ship Information")):SetFont("HEADING_LARGE"),
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label(t("HYPERDRIVE")..":"),
								ui:Label(t("HYPERSPACE_RANGE")..":"),
								ui:Margin(10),
								ui:Label(t("Weight empty:")),								
								ui:Label(t("CAPACITY_USED")..":"),
								ui:Label(t("FUEL_WEIGHT")..":"),
								ui:Label(t("TOTAL_WEIGHT")..":"),
								ui:Margin(10),
								ui:Label(t("FRONT_WEAPON")..":"),
								ui:Label(t("REAR_WEAPON")..":"),
								ui:Label(t("FUEL")..":"),
							})
						})
						:SetColumn(1, {
							ui:VBox():PackEnd({
								ui:Label(EquipType.GetEquipType(hyperdrive).name),
								ui:Label(string.interp(
									t("{range} light years ({maxRange} max)"), {
										range    = string.format("%.1f",stats.hyperspaceRange),
										maxRange = string.format("%.1f",stats.maxHyperspaceRange)
									}
								)),
								ui:Margin(10),
								ui:Label(string.format("%dt", stats.totalMass - stats.usedCapacity)),								
								ui:Label(string.format("%dt (%dt free)", stats.usedCapacity,  stats.freeCapacity)),
								ui:Label(string.format("%dt (%dt max)", math.floor(Game.player.fuel/100*stats.maxFuelTankMass + 0.5), stats.maxFuelTankMass )),
								ui:Label(string.format("%dt", math.floor(stats.totalMass+Game.player.fuel/100*stats.maxFuelTankMass + 0.5) )),
								ui:Margin(10),
								ui:Label(EquipType.GetEquipType(frontWeapon).name),
								ui:Label(EquipType.GetEquipType(rearWeapon).name),
								ui:Label(string.format("%d%%", Game.player.fuel)),
							})
						}),
					ui:Label(t("Equipment")):SetFont("HEADING_NORMAL"),
					ui:Grid(2,1)
						:SetColumn(0, { ui:VBox():PackEnd(equipColumn[1]) })
						:SetColumn(1, { ui:VBox():PackEnd(equipColumn[2]) })
				})
			})
			:SetColumn(1, {
				ui:VBox(10)
					:PackEnd(ui:Label(shipType.name):SetFont("HEADING_LARGE"))
					:PackEnd(UI.Game.ShipSpinner.New(ui, Game.player.flavour))
			})
end

local orbitalAnalysis = function ()
	local orbitalBody -- What we, or our space station, are orbiting
	local frameBody = Game.player.frameBody
	if not frameBody then return end -- Bug out if we're in a null frame. Save an embarrassing crash.
	if frameBody.superType == 'STARPORT' then
		orbitalBody = Space.GetBody(frameBody.path:GetSystemBody().parent.index)
	else
		orbitalBody = frameBody
	end
	
	local distance = Game.player:DistanceTo(orbitalBody)
	local mass = orbitalBody.path:GetSystemBody().mass
	local radius = orbitalBody.path:GetSystemBody().radius
	local name = orbitalBody.label

	local G = 6.67428e-11

	local vCircular = math.sqrt((G * mass)/distance)
	local vEscape = math.sqrt((2 * G * mass)/distance)
	local vDescent = math.sqrt(G * mass * ((2 / distance) - (2 / (distance + radius))))

	return ui:Expand():SetInnerWidget(
		ui:VBox(20):PackEnd({
			ui:Label(t('Orbital Analysis')):SetFont('HEADING_LARGE'),
			ui:Label((t('Located {distance}km from the centre of {name}:')):interp({
														-- convert to kilometres
														distance = string.format('%6.2f',distance/1000),
														name = name
													})),
			ui:Grid(2,1)
				:SetColumn(0, {
					ui:VBox():PackEnd({
						ui:Label(t('Circular orbit speed:')),
						ui:Label(t('Escape speed:')),
						ui:Label(t('Descent-to-ground speed:')),
					})
				})
				:SetColumn(1, {
					ui:VBox():PackEnd({
						-- convert to kilometres per second
						ui:Label(string.format('%6.2fkm/s',vCircular/1000)),
						ui:Label(string.format('%6.2fkm/s',vEscape/1000)),
						ui:Label(string.format('%6.2fkm/s',vDescent/1000)),
					})
				}),
			ui:MultiLineText((t('ORBITAL_ANALYSIS_NOTES')):interp({name = name}))
		})
	)

end

local personalInfo = function ()
	local player = PersistentCharacters.player
	local faceFlags = { player.female and "FEMALE" or "MALE" }

	-- for updating the caption
	local faceWidget = uilib.FaceWidget(player)
	-- for updating the entire face
	local faceWidgetContainer = ui:Margin(0):SetInnerWidget(faceWidget)

	local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
	nameEntry.onEnter:Connect(function (newName)
		print(newName)
		player.name = newName
		uilib.UpdateFaceText(faceWidget,player)
	end )

	local genderLabel = ui:Label(player.female and t("Female") or t("Male"))
	local genderSelector = ui:Button():SetInnerWidget(genderLabel)
	genderSelector.onClick:Connect(function ()
		player.female = not player.female
		faceWidget = uilib.FaceWidget(player)
		faceWidgetContainer:SetInnerWidget(faceWidget)
		genderLabel:SetText(player.female and t("Female") or t("Male"))
	end)

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Label(t("Personal Information")):SetFont("HEADING_LARGE"),
					ui:Label(t("Combat")):SetFont("HEADING_NORMAL"),
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label(t("Rating:")),
								ui:Label(t("Kills:")),
							})
						})
						:SetColumn(1, {
							ui:VBox():PackEnd({
								ui:Label(t(player:GetCombatRating())),
								ui:Label(string.format('%d',player.killcount)),
							})
						}),
					ui:Label(t("Military")):SetFont("HEADING_NORMAL"),
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label(t("ALLEGIANCE")),
								ui:Label(t("Rank:")),
							})
						})
						:SetColumn(1, {
							ui:VBox():PackEnd({
								ui:Label(t('NONE')), -- XXX
								ui:Label(t('NONE')), -- XXX
							})
						})
				})
			})
			:SetColumn(1, {
				ui:VBox(10)
					:PackEnd(ui:HBox()
						:PackEnd(nameEntry)
						:PackEnd(genderSelector)
					)
					:PackEnd(faceWidgetContainer)
			})
end

local econTrade = function ()

	local cash = Game.player:GetMoney()

	local stats = Game.player:GetStats()

	local usedCargo = stats.usedCargo
	local totalCargo = stats.freeCapacity

	local usedCabins = Game.player:GetEquipCount("CABIN", "PASSENGER_CABIN")
	local totalCabins = Game.player:GetEquipCount("CABIN", "UNOCCUPIED_CABIN") + usedCabins

	-- Using econTrade as an enclosure for the functions attached to the
	-- buttons in the UI object that it returns. Seems like the most sane
	-- way to handle it; hopefully the enclosure will evaporate shortly
	-- after the UI is disposed of.

	updateCargoListWidget = function ()

		local cargoNameColumn = {}
		local cargoQuantityColumn = {}
		for i = 1,#Constants.EquipType do
			local type = Constants.EquipType[i]
			if type ~= "NONE" then
				local et = EquipType.GetEquipType(type)
				local slot = et.slot
				if slot == "CARGO" then
					local count = Game.player:GetEquipCount(slot, type)
					if count > 0 then
						table.insert(cargoNameColumn, ui:Label(et.name))
						table.insert(cargoQuantityColumn, ui:Label(count.."t"))
					end
				end
			end
		end

		-- Function returns a UI with which to populate the cargo list widget
		return
			ui:VBox(10):PackEnd({
				ui:Label(t("CARGO")):SetFont("HEADING_LARGE"),
				ui:Scroller():SetInnerWidget(
					ui:Grid(2,1)
						:SetColumn(0, { ui:VBox():PackEnd(cargoNameColumn) })
						:SetColumn(1, { ui:VBox():PackEnd(cargoQuantityColumn) })
				)
			})
	end

	-- Make a cargo list widget that we can revisit and update
	local cargoListWidget = ui:Margin(0)
		:SetInnerWidget(updateCargoListWidget())

	local totalCargoWidget = ui:Label(t("Total: ")..totalCargo.."t")
	local usedCargoWidget = ui:Label(t("USED")..": "..usedCargo.."t")

	local refuel = function ()
		-- UI button where the player clicks to refuel...
		Game.player:Refuel(1)
		-- ...then we update the cargo list widget...
		cargoListWidget:SetInnerWidget(updateCargoListWidget())
		-- ...and the totals.
		stats = Game.player:GetStats()
		totalCargoWidget:SetText(t("Total: ")..stats.freeCapacity.."t")
		usedCargoWidget:SetText(t("USED")..": "..stats.usedCargo.."t")
	end

	-- Define the refuel button
	if Game.player:GetEquipCount('CARGO', 'WATER') > 0 then
		refuelButton = ui:Button():SetInnerWidget(ui:Label(t('REFUEL')))
		refuelButton.onClick:Connect(refuel)
	end

	return ui:Expand():SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Label(t("Economy & Trade")):SetFont("HEADING_LARGE"),
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label(t("CASH")..":"),
								ui:Margin(10),
								ui:Label(t("CARGO_SPACE")..":"),
								ui:Label(t("CABINS")..":"),
								refuelButton,
							})
						})
						:SetColumn(1, {
							ui:VBox():PackEnd({
								ui:Label(string.format("$%.2f", cash)),
								ui:Margin(10),
								ui:Grid(2,1):SetRow(0, { totalCargoWidget, usedCargoWidget }),
								ui:Grid(2,1):SetRow(0, { ui:Label(t("Total: ")..totalCabins), ui:Label(t("USED")..": "..usedCabins) }),
							})
						}),
				})
			})
			:SetColumn(1, {
				cargoListWidget
			})
	)
end

local missions = function ()
	-- This mission screen
	local MissionScreen = ui:Expand()
	local MissionList = ui:VBox(10)

	if #PersistentCharacters.player.missions == 0 then
		MissionList
			:PackEnd({
				ui:Label(t("MISSIONS")):SetFont("HEADING_LARGE"),
				ui:Label(t("No missions."))
			})

		return MissionScreen:SetInnerWidget(MissionList)
	end

	-- One row for each mission, plus a header
	local headergrid  = ui:Grid(6,1)
	local missiongrid = ui:Grid(6,#PersistentCharacters.player.missions)

	headergrid:SetRow(0,
	{
		-- Headers
		ui:Label(t('TYPE')),
		ui:Label(t('CLIENT')),
		ui:Label(t('LOCATION')),
		ui:Label(t('DUE')),
		ui:Label(t('REWARD')),
		ui:Label(t('STATUS')),
	})

	local count = 0 -- We need to count rows, can't rely on table keys
	for ref,mission in ipairs(PersistentCharacters.player.missions) do
		-- Format the location
		local missionLocationName
		if mission.location.bodyIndex then
			missionLocationName = string.format('%s, %s [%d,%d,%d]', mission.location:GetSystemBody().name, mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		else
			missionLocationName = string.format('%s [%d,%d,%d]', mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		end
        -- Decide what happens when the button's clicked
		local button = ui:Button():SetInnerWidget(ui:HBox():PackEnd(ui:Label(t(mission.status))))
		local clickHandler = function ()
			MissionScreen:SetInnerWidget(ui:VBox()
				:PackEnd({ui:Label(t('Mission Details')):SetFont('HEADING_LARGE')})
				:PackEnd((Mission.GetClick(mission.type))(ref)))
		end
		button.onClick:Connect(clickHandler)
		missiongrid:SetRow(count,{
			ui:Label(t(mission.type)),
			ui:Label(mission.client.name),
			ui:Label(missionLocationName),
			ui:Label(Format.Date(mission.due)),
			ui:Label(Format.Money(mission.reward)),
			button,
		})
		count = count + 1
	end

	MissionList
			:PackEnd({
				ui:Label(t("MISSIONS")):SetFont("HEADING_LARGE"),
				headergrid
			})
			:PackEnd(ui:Scroller():SetInnerWidget(missiongrid))

	MissionScreen:SetInnerWidget(MissionList)

	return MissionScreen
end

ui.templates.InfoView = function (args)
	local buttonDefs = {
		{ t("Ship Information"),     shipInfo },
		{ t("Personal Information"), personalInfo },
		{ t("Economy & Trade"),      econTrade },
		{ t("MISSIONS"),             missions },
		{ t('Orbit'),                orbitalAnalysis },
    }

	local container = ui:Margin(30)

    local buttonSet = {}
    for i = 1,#buttonDefs do
        local def = buttonDefs[i]
        local b = ui:Button():SetInnerWidget(ui:HBox():PackEnd(ui:Label(def[1])))
        b.onClick:Connect(function () container:SetInnerWidget(def[2]()) end)
        table.insert(buttonSet, ui:Margin(2):SetInnerWidget(b))
    end

	container:SetInnerWidget(shipInfo())

	return
		ui:VBox()
			:PackEnd(
				ui:Grid(#buttonDefs,1):SetFont("HEADING_NORMAL")
					:SetRow(0, buttonSet))
			:PackEnd(
				ui:Background():SetInnerWidget(container))
end

