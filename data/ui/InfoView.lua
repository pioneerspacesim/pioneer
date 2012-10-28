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
			ui:Label((t('ORBITAL_ANALYSIS_NOTES')):interp({name = name}))
		})
	)

end

local personalInfo = function ()
	local player = PersistentCharacters.player
	local faceFlags = { player.female and "FEMALE" or "MALE" }

	local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
	nameEntry.onEnter:Connect(function (newName) print(newName) player.name = newName end )

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
					:PackEnd(nameEntry)
					:PackEnd(UI.Game.Face.New(ui, faceFlags, player.seed))
			})
end

local econTrade = function ()
	local cash = Game.player:GetMoney()

	local stats = Game.player:GetStats()

	local usedCargo = stats.usedCargo
	local totalCargo = stats.freeCapacity

	local usedCabins = Game.player:GetEquipCount("CABIN", "PASSENGER_CABIN")
	local totalCabins = Game.player:GetEquipCount("CABIN", "UNOCCUPIED_CABIN") + usedCabins

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
							})
						})
						:SetColumn(1, {
							ui:VBox():PackEnd({
								ui:Label(string.format("$%.2f", cash)),
								ui:Margin(10),
								ui:Grid(2,1):SetRow(0, { ui:Label(t("Total: ")..totalCargo.."t"), ui:Label(t("USED")..": "..usedCargo.."t") }),
								ui:Grid(2,1):SetRow(0, { ui:Label(t("Total: ")..totalCabins), ui:Label(t("USED")..": "..usedCabins) }),
							})
						}),
				})
			})
			:SetColumn(1, {
				ui:VBox(10):PackEnd({
					ui:Label(t("CARGO")):SetFont("HEADING_LARGE"),
					ui:Scroller():SetInnerWidget(
						ui:Grid(2,1)
							:SetColumn(0, { ui:VBox():PackEnd(cargoNameColumn) })
							:SetColumn(1, { ui:VBox():PackEnd(cargoQuantityColumn) })
					)
				})
			})
	)
end

local missions = function ()
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
			(({
				ACTIVE = Mission.GetClick(mission.type),
				COMPLETED = function (ref)
								Mission.Remove(ref)
							end,
				FAILED = function (ref) return end,
			})[mission.status])(ref)
		end
		button.onClick:Connect(clickHandler)
		missiongrid:SetRow(count,{
			ui:Label(t(mission.type)),
			ui:Label(mission.client),
			ui:Label(missionLocationName),
			ui:Label(Format.Date(mission.due)),
			ui:Label(Format.Money(mission.reward)),
			-- To do: Missions can hook code to this button, so that the player can
			-- read the brief whilst in flight, or get access to whatever information,
			-- interactive or not, necessary for the mission.
			button,
		})
		count = count + 1
	end

	return
		ui:VBox(10)
			:PackEnd({
				ui:Label(t("MISSIONS")):SetFont("HEADING_LARGE"),
				headergrid
			})
			:PackEnd(ui:Scroller():SetInnerWidget(missiongrid))
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

