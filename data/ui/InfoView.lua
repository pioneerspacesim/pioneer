local ui = Engine.ui
local t = Translate:GetTranslator()

local shipInfo = function (args)
	local shipId = Game.player.shipId
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
					ui:Label(t("Equipment")):SetFont("HEADING_LARGE"),
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
	if not frameBody then
		-- Bug out if we're in a null frame. Save an embarrassing crash.
		return ui:Label(t('FAILED'))
	end
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
	local faceWidget = UI.InfoFace.New(player)
	-- for updating the entire face
	local faceWidgetContainer = ui:Margin(0, "ALL", faceWidget)

	local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
	nameEntry.onChange:Connect(function (newName)
		player.name = newName
        faceWidget:UpdateInfo(player)
	end )

	local genderToggle = UI.SmallLabeledButton.New("Toggle male/female")
	genderToggle.button.onClick:Connect(function ()
		player.female = not player.female
		faceWidget = UI.InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	local generateFaceButton = UI.SmallLabeledButton.New("Make new face")
	generateFaceButton.button.onClick:Connect(function ()
		player.seed = Engine.rand:Integer()
		faceWidget = UI.InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Label(t("Combat")):SetFont("HEADING_LARGE"),
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
					ui:Label(t("Military")):SetFont("HEADING_LARGE"),
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
					:PackEnd(ui:HBox(10):PackEnd({
						ui:VBox(5):PackEnd({
							ui:Expand("HORIZONTAL", nameEntry),
						}),
						ui:VBox(5):PackEnd({
							genderToggle,
							generateFaceButton,
						})
					}))
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

	-- Make a cargo list widget that we can revisit and update
	local cargoListWidget = ui:Margin(0)

	function updateCargoListWidget ()

		local cargoNameColumn = {}
		local cargoQuantityColumn = {}
		local cargoJettisonColumn = {}

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

						local jettisonButton = UI.SmallLabeledButton.New(t("JETTISON"))
						jettisonButton.button.onClick:Connect(function ()
							Game.player:Jettison(type)
							updateCargoListWidget()
							cargoListWidget:SetInnerWidget(updateCargoListWidget())
						end)
						table.insert(cargoJettisonColumn, jettisonButton.widget)
					end
				end
			end
		end

		-- Function returns a UI with which to populate the cargo list widget
		return
			ui:VBox(10):PackEnd({
				ui:Label(t("CARGO")):SetFont("HEADING_LARGE"),
				ui:Scroller():SetInnerWidget(
					ui:Grid(3,1)
						:SetColumn(0, { ui:VBox():PackEnd(cargoNameColumn) })
						:SetColumn(1, { ui:VBox():PackEnd(cargoQuantityColumn) })
						:SetColumn(2, { ui:VBox():PackEnd(cargoJettisonColumn) })
				)
			})
	end

	cargoListWidget:SetInnerWidget(updateCargoListWidget())

	local totalCargoWidget = ui:Label(t("Total: ")..totalCargo.."t")
	local usedCargoWidget = ui:Label(t("USED")..": "..usedCargo.."t")

	-- Define the refuel button
	local refuelButton = UI.SmallLabeledButton.New(t('REFUEL'))

	local refuelButtonRefresh = function ()
		if Game.player.fuel == 100 or Game.player:GetEquipCount('CARGO', 'WATER') == 0 then refuelButton.widget:Disable() end
	end
	refuelButtonRefresh()

	local refuel = function ()
		-- UI button where the player clicks to refuel...
		Game.player:Refuel(1)
		-- ...then we update the cargo list widget...
		cargoListWidget:SetInnerWidget(updateCargoListWidget())
		-- ...and the totals.
		stats = Game.player:GetStats()
		totalCargoWidget:SetText(t("Total: ")..stats.freeCapacity.."t")
		usedCargoWidget:SetText(t("USED")..": "..stats.usedCargo.."t")

		refuelButtonRefresh()
	end

	refuelButton.button.onClick:Connect(refuel)

	return ui:Expand():SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(20):PackEnd({
					ui:Grid(2,1)
						:SetColumn(0, {
							ui:VBox():PackEnd({
								ui:Label(t("CASH")..":"),
								ui:Margin(10),
								ui:Label(t("CARGO_SPACE")..":"),
								ui:Label(t("CABINS")..":"),
								ui:Margin(10),
								refuelButton.widget,
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
		MissionList:PackEnd( ui:Label(t("No missions.")) )

		return MissionScreen:SetInnerWidget(MissionList)
	end

	-- One row for each mission, plus a header
	local rowspec = {7,8,10,8,5,5,5}
	local headergrid  = ui:Grid(rowspec,1)

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

	local missionbox = ui:VBox(10)

	for ref,mission in pairs(PersistentCharacters.player.missions) do
		-- Format the location
		local missionLocationName
		if mission.location.bodyIndex then
			missionLocationName = string.format('%s, %s [%d,%d,%d]', mission.location:GetSystemBody().name, mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		else
			missionLocationName = string.format('%s [%d,%d,%d]', mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		end

		local moreButton = UI.SmallLabeledButton.New("More info...")
		moreButton.button.onClick:Connect(function ()
			MissionScreen:SetInnerWidget(ui:VBox(10)
				:PackEnd({ui:Label(t('Mission Details')):SetFont('HEADING_LARGE')})
				:PackEnd((mission:GetClick())(mission)))
		end)

		local description = mission:GetTypeDescription()
		missionbox:PackEnd(ui:Grid(rowspec,1):SetRow(0, {
			ui:Label(description or t('NONE')),
			ui:Label(mission.client.name),
			ui:MultiLineText(missionLocationName),
			ui:Label(Format.Date(mission.due)),
			ui:Label(Format.Money(mission.reward)),
			-- nil description means mission type isn't registered.
			ui:Label((description and t(mission.status)) or t('INACTIVE')),
			moreButton.widget,
		}))
	end

	MissionList:PackEnd({ headergrid, ui:Scroller():SetInnerWidget(missionbox) })

	MissionScreen:SetInnerWidget(MissionList)

	return MissionScreen
end

local tabGroup
ui.templates.InfoView = function (args)
	if tabGroup then
		tabGroup:SwitchFirst()
		return tabGroup.widget
	end

	tabGroup = UI.TabGroup.New()

	tabGroup:AddTab({ id = "shipInfo",        title = t("Ship Information"),     icon = "Satellite", template = shipInfo         })
	tabGroup:AddTab({ id = "personalInfo",    title = t("Personal Information"), icon = "User",      template = personalInfo     })
	tabGroup:AddTab({ id = "econTrade",       title = t("Economy & Trade"),      icon = "Cart",      template = econTrade,       })
	tabGroup:AddTab({ id = "missions",        title = t("MISSIONS"),             icon = "Star",      template = missions,        })
	--tabGroup:AddTab({ id = "orbitalAnalysis", title = t("Orbital Analysis"),     icon = "Planet",    template = orbitalAnalysis, })

	return tabGroup.widget
end

