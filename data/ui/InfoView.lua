local ui = Engine.ui
local t = Translate:GetTranslator()

local shipInfo = function (args)
	local shipDef = ShipDef[Game.player.shipId]

	local hyperdrive =              table.unpack(Game.player:GetEquip("ENGINE"))
	local frontWeapon, rearWeapon = table.unpack(Game.player:GetEquip("LASER"))

	hyperdrive =  hyperdrive  or "NONE"
	frontWeapon = frontWeapon or "NONE"
	rearWeapon =  rearWeapon  or "NONE"

	local stats = Game.player:GetStats()

	local equipItems = {}
	for i = 1,#Constants.EquipType do
		local type = Constants.EquipType[i]
		local et = EquipDef[type]
		local slot = et.slot
		if (slot ~= "CARGO" and slot ~= "MISSILE" and slot ~= "ENGINE" and slot ~= "LASER") then
			local count = Game.player:GetEquipCount(slot, type)
			if count > 0 then
				if count > 1 then
					if type == "SHIELD_GENERATOR" then
						table.insert(equipItems,
							ui:Label(string.interp(t("{quantity} Shield Generators"), { quantity = string.format("%d", count) })))
					elseif type == "PASSENGER_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(t("{quantity} Occupied Passenger Cabins"), { quantity = string.format("%d", count) })))
					elseif type == "UNOCCUPIED_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(t("{quantity} Unoccupied Passenger Cabins"), { quantity = string.format("%d", count) })))
					else
						table.insert(equipItems, ui:Label(et.name))
					end
				else
					table.insert(equipItems, ui:Label(et.name))
				end
			end
		end
	end

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("HYPERDRIVE")..":", EquipDef[hyperdrive].name },
						{
							t("HYPERSPACE_RANGE")..":",
							string.interp(
								t("{range} light years ({maxRange} max)"), {
									range    = string.format("%.1f",stats.hyperspaceRange),
									maxRange = string.format("%.1f",stats.maxHyperspaceRange)
								}
							),
						},
						"",
						{ t("Weight empty:"),      string.format("%dt", stats.totalMass - stats.usedCapacity) },
						{ t("CAPACITY_USED")..":", string.format("%dt (%dt "..t("free")..")", stats.usedCapacity,  stats.freeCapacity) },
						{ t("FUEL_WEIGHT")..":",   string.format("%dt (%dt "..t("max")..")", math.floor(Game.player.fuel/100*stats.maxFuelTankMass + 0.5), stats.maxFuelTankMass ) },
						{ t("TOTAL_WEIGHT")..":",  string.format("%dt", math.floor(stats.totalMass+Game.player.fuel/100*stats.maxFuelTankMass + 0.5) ) },
						"",
						{ t("FRONT_WEAPON")..":", EquipDef[frontWeapon].name },
						{ t("REAR_WEAPON")..":",  EquipDef[rearWeapon].name },
						{ t("FUEL")..":",         string.format("%d%%", Game.player.fuel) },
						"",
						{ t("Minimum crew")..":", ShipType.GetShipType(Game.player.shipId).minCrew },
						{ t("Crew cabins")..":",  ShipType.GetShipType(Game.player.shipId).maxCrew },
					}),
					"",
					ui:Label(t("Equipment")):SetFont("HEADING_LARGE"),
					ui:Table():AddRows(equipItems),
				})
			})
			:SetColumn(1, {
				ui:VBox(10)
					:PackEnd(ui:Label(shipDef.name):SetFont("HEADING_LARGE"))
					:PackEnd(UI.Game.ModelSpinner.New(ui, shipDef.modelName, Game.player:GetSkin()))
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
            (t('Located {distance}km from the centre of {name}:')):interp({
														-- convert to kilometres
														distance = string.format('%6.2f',distance/1000),
														name = name
													}),
			ui:Table():SetColumnSpacing(10):AddRows({
				-- convert to kilometres per second
				{ t('Circular orbit speed:'),    string.format('%6.2fkm/s',vCircular/1000) },
				{ t('Escape speed:'),            string.format('%6.2fkm/s',vEscape/1000)   },
				{ t('Descent-to-ground speed:'), string.format('%6.2fkm/s',vDescent/1000)  },
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

	local genderToggle = UI.SmallLabeledButton.New(t("Toggle male/female"))
	genderToggle.button.onClick:Connect(function ()
		player.female = not player.female
		faceWidget = UI.InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	local generateFaceButton = UI.SmallLabeledButton.New(t("Make new face"))
	generateFaceButton.button.onClick:Connect(function ()
		player.seed = Engine.rand:Integer()
		faceWidget = UI.InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Label(t("Combat")):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("Rating:"), t(player:GetCombatRating()) },
						{ t("Kills:"),  string.format('%d',player.killcount) },
					}),
					"",
					ui:Label(t("Military")):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("ALLEGIANCE"), t('NONE') }, -- XXX
						{ t("Rank:"),      t('NONE') }, -- XXX
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
				local et = EquipDef[type]
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

	local cargoGauge = UI.InfoGauge.New({
		formatter = function (v)
			local stats = Game.player:GetStats()
			return string.format("%d/%dt", stats.usedCargo, stats.freeCapacity)
		end
	})
	cargoGauge:SetValue(stats.usedCargo/stats.freeCapacity)

	local fuelGauge = UI.InfoGauge.New({
		label          = ui:NumberLabel("PERCENT"),
		warningLevel   = 0.1,
		criticalLevel  = 0.05,
		levelAscending = false,
	})
	fuelGauge.label:Bind("valuePercent", Game.player, "fuel")
	fuelGauge.gauge:Bind("valuePercent", Game.player, "fuel")

	-- Define the refuel button
	local refuelButton = UI.SmallLabeledButton.New(t('REFUEL'))

	local refuelButtonRefresh = function ()
		if Game.player.fuel == 100 or Game.player:GetEquipCount('CARGO', 'WATER') == 0 then refuelButton.widget:Disable() end
		fuelGauge:SetValue(Game.player.fuel/100)
	end
	refuelButtonRefresh()

	local refuel = function ()
		-- UI button where the player clicks to refuel...
		Game.player:Refuel(1)
		-- ...then we update the cargo list widget...
		cargoListWidget:SetInnerWidget(updateCargoListWidget())
		-- ...and the gauge.
		stats = Game.player:GetStats()
		cargoGauge:SetValue(stats.usedCargo/stats.freeCapacity)

		refuelButtonRefresh()
	end

	refuelButton.button.onClick:Connect(refuel)

	return ui:Expand():SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Margin(5, "HORIZONTAL",
					ui:VBox(20):PackEnd({
						ui:Grid(2,1)
							:SetColumn(0, {
								ui:VBox():PackEnd({
									ui:Label(t("CASH")..":"),
									ui:Margin(10),
									ui:Label(t("CARGO_SPACE")..":"),
									ui:Label(t("CABINS")..":"),
									ui:Margin(10),
								})
							})
							:SetColumn(1, {
								ui:VBox():PackEnd({
									ui:Label(string.format("$%.2f", cash)),
									ui:Margin(10),
									cargoGauge.widget,
									ui:Grid(2,1):SetRow(0, { ui:Label(t("Total: ")..totalCabins), ui:Label(t("USED")..": "..usedCabins) }),
									ui:Margin(10),
								})
							}),
						ui:Grid({50,10,40},1)
							:SetRow(0, {
								ui:HBox(5):PackEnd({
									ui:Label("Fuel:"),
									fuelGauge,
								}),
								nil,
								refuelButton.widget,
							})
					})
				)
			})
			:SetColumn(1, {
				cargoListWidget
			})
	)
end

-- we keep MissionList to remember players preferences
-- (now it is column he wants to sort by)
local MissionList 
local missions = function (tabGroup)
	-- This mission screen
	local MissionScreen = ui:Expand()

	if #PersistentCharacters.player.missions == 0 then
		return MissionScreen:SetInnerWidget( ui:Label(t("No missions.")) )
	end

	local rowspec = {7,8,9,9,5,5,5} -- 7 columns
	if MissionList then 
		MissionList:Clear()
	else
		MissionList = UI.SmartTable.New(rowspec) 
	end
	
	-- setup headers
	local headers = 
	{
		t("TYPE"),
		t("CLIENT"),
		t("LOCATION"),
		t("DUE"),
		t("REWARD"),
		t("STATUS"),
	}
	MissionList:SetHeaders(headers)
	
	-- we're not happy with default sort function so we specify one by ourselves
	local sortMissions = function (misList)
		local col = misList.sortCol
		local cmpByReward = function (a,b) 
			return a.data[col] >= b.data[col] 
		end
		local comparators = 
		{ 	-- by column num
			[5]	= cmpByReward,
		}
		misList:defaultSortFunction(comparators[col])
	end
	MissionList:SetSortFunction(sortMissions)

	for ref,mission in pairs(PersistentCharacters.player.missions) do
		-- Format the location
		local missionLocationName
		if mission.location.bodyIndex then
			missionLocationName = string.format('%s, %s [%d,%d,%d]', mission.location:GetSystemBody().name, mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		else
			missionLocationName = string.format('%s [%d,%d,%d]', mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		end
		-- Format the distance label
		local playerSystem = Game.system or Game.player:GetHyperspaceTarget()
		local dist = playerSystem:DistanceTo(mission.location)
		local distLabel = ui:Label(string.format('%.2f %s', dist, t('ly')))
		local hyperjumpStatus = Game.player:GetHyperspaceDetails(mission.location)
		if hyperjumpStatus == 'CURRENT_SYSTEM' then
			distLabel:SetColor({ r = 0.0, g = 1.0, b = 0.2 }) -- green
		else
			if hyperjumpStatus == 'OK' then
				distLabel:SetColor({ r = 1.0, g = 1.0, b = 0.0 }) -- yellow
			else
				distLabel:SetColor({ r = 1.0, g = 0.0, b = 0.0 }) -- red
			end
		end
		-- Pack location and distance
		local locationBox = ui:VBox(2):PackEnd(ui:MultiLineText(missionLocationName))
									  :PackEnd(distLabel)
		
		-- Format Due info
		local dueLabel = ui:Label(Format.Date(mission.due))
		local days = math.max(0, (mission.due - Game.time) / (24*60*60))
		local daysLabel = ui:Label(string.format(t("%d days left"), days)):SetColor({ r = 1.0, g = 0.0, b = 1.0 }) -- purple
		local dueBox = ui:VBox(2):PackEnd(dueLabel):PackEnd(daysLabel)
		
		local moreButton = UI.SmallLabeledButton.New(t("More info..."))
		moreButton.button.onClick:Connect(function ()
			MissionScreen:SetInnerWidget(ui:VBox(10)
				:PackEnd({ui:Label(t('Mission Details')):SetFont('HEADING_LARGE')})
				:PackEnd((mission:GetClick())(mission)))
		end)

		local description = mission:GetTypeDescription()
		local row =
		{ -- if we don't specify widget, default one will be used 
			{data = description or t('NONE')},
			{data = mission.client.name},
			{data = dist, widget = locationBox},
			{data = mission.due, widget = dueBox},
			{data = mission.reward, widget = ui:Label(Format.Money(mission.reward)):SetColor({ r = 0.0, g = 1.0, b = 0.2 })}, -- green
			-- nil description means mission type isn't registered.
			{data = (description and t(mission.status)) or t('INACTIVE')},
			{widget = moreButton.widget}
		}
		MissionList:AddRow(row)
	end

	MissionScreen:SetInnerWidget(ui:Scroller(MissionList))

	return MissionScreen
end

-- Anti-abuse feature - this locks out the piloting commands based on a timer.
-- It knows when the crew were last checked for a piloting skill, and prevents
-- the player drumming the button until it works.
local pilotLockoutTimer = 0
local pilotLockoutTimeout = 30 -- Half a minute (in seconds)

local checkPilotLockout = function ()
	return Game.time > pilotLockoutTimer + pilotLockoutTimeout
end

local pilotLockout = function ()
	pilotLockoutTimer = Game.time
end


local crewRoster = function ()
	-- This Crew Roster screen
	local CrewScreen = ui:Expand()

	-- Prototype for a function; makeCrewList and crewMemberInfoButtonFunc
	-- can call each other.
	local crewMemberInfoButtonFunc

	-- Function that presents a screen with orders to be given to the crew.
	-- The crew will each be tested in turn for suitability, and the first
	-- to respond well will be tasked with the job.
	local taskCrew = function ()
		local feedback = ui:Label('') -- Would prefer MultiLineText

		-- Very local function used by functions in crewTasks
		local testCrewMember = function (attribute,playerOK)
			for crewMember in Game.player:EachCrewMember() do
				local result = crewMember:TestRoll(attribute)
				if (playerOK or not crewMember.player) and result then
					return crewMember,result
				end
			end
			return false
		end

		-- A table of task functions, keyed by their description, which will set feedback.
		-- They take no arguments, and are connected to buttons.
		local crewTasks = {
			['Attempt to repair hull'] = function ()
				-- Convoluted...
				local hullMass = ShipType.GetShipType(Game.player.shipId).hullMass
				local hullMassLeft = Game.player:GetStats().hullMassLeft 
				local hullDamage = hullMass - hullMassLeft
				if hullDamage > 0 then
					if Game.player:GetEquipCount('CARGO','METAL_ALLOYS') <= 0 then
						feedback:SetText(t('Not enough {alloy} to attempt a repair'):interp({alloy = t('METAL_ALLOYS')}))
						return
					end
					local crewMember, result = testCrewMember('engineering',true)
					if crewMember then
						local repair = math.min(
							-- Need metal alloys for repair. Check amount.
							math.ceil(hullDamage/(64 - result)), -- 65 > result > 3
							Game.player:GetEquipCount('CARGO','METAL_ALLOYS')
						)
						Game.player:RemoveEquip('METAL_ALLOYS',repair) -- These will now be part of the hull.
						repairPercent = math.min(math.ceil(100 * (repair + hullMassLeft) / hullMass), 100) -- Get new hull percentage...
						Game.player:SetHullPercent(repairPercent)   -- ...and set it.
						feedback:SetText(t('Hull repaired by {name}, now at {repairPercent}%'):interp({name = crewMember.name,repairPercent = repairPercent}))
					else
						repairPercent = math.max(math.floor(100 * (hullMassLeft - 1) / hullMass), 1) -- Get new hull percentage...
						Game.player:SetHullPercent(repairPercent)   -- ...and set it.
						feedback:SetText(t('Hull repair attempt failed. Hull suffered minor damage.'))
					end
				else
					feedback:SetText(t('Hull does not require repair.'))
				end
			end,

			['Destroy enemy ship'] = function ()
				if Game.player.flightState ~= 'FLYING'
				then
					feedback:SetText(({
						DOCKED = t('You must request launch clearance first, Commander.'),
						LANDED = t('You must launch first, Commander.'),
						HYPERSPACE = t('We are in hyperspace, Commander.'),
						DOCKING = t('The ship is under station control, Commander.'),
					})[Game.player.flightState])
				elseif not Game.player:GetCombatTarget() then
					feedback:SetText(t('You must first select a combat target, Commander.'))
				else
					local crewMember = checkPilotLockout() and testCrewMember('piloting')
					if not crewMember then
						feedback:SetText(t('There is nobody else on board able to fly this ship.'))
						pilotLockout()
					else
						feedback:SetText(t('Pilot seat is now occupied by {name}'):interp({name = crewMember.name,repairPercent = repairPercent}))
						Game.player:AIKill(Game.player:GetCombatTarget())
					end
				end
			end,

			['Dock at current target'] = function ()
				local target = Game.player:GetNavTarget()
				if Game.player.flightState ~= 'FLYING'
				then
					feedback:SetText(({
						DOCKED = t('You must request launch clearance first, Commander.'),
						LANDED = t('You must launch first, Commander.'),
						HYPERSPACE = t('We are in hyperspace, Commander.'),
						DOCKING = t('The ship is under station control, Commander.'),
					})[Game.player.flightState])
				elseif not (target and target:isa('SpaceStation')) then
					feedback:SetText(t('You must first select a suitable navigation target, Commander.'))
				else
					local crewMember = checkPilotLockout() and testCrewMember('piloting')
					if not crewMember then
						feedback:SetText(t('There is nobody else on board able to fly this ship.'))
						pilotLockout()
					else
						feedback:SetText(t('Pilot seat is now occupied by {name}'):interp({name = crewMember.name,repairPercent = repairPercent}))
						Game.player:AIDockWith(target)
					end
				end
			end,
		}

		local taskList = ui:VBox() -- This could do with being something prettier

		for label,task in pairs(crewTasks) do
			local taskButton = UI.SmallLabeledButton.New(t(label))
			taskButton.button.onClick:Connect(task)
			taskList:PackEnd(taskButton)
		end
		taskList:PackEnd(feedback)

		CrewScreen:SetInnerWidget(taskList)
	end

	-- Function that creates the crew list
	local makeCrewList = function ()
		local crewlistbox = ui:VBox(10)

		-- One row for each mission, plus a header
		local rowspec = {8,8,4,4,7,5}
		local headergrid  = ui:Grid(rowspec,1)

		-- Set up the headings for the Crew Roster list
		headergrid:SetRow(0,
		{
			-- Headers
			ui:Label(t('Name')):SetFont("HEADING_NORMAL"),
			ui:Label(t('Position')):SetFont("HEADING_NORMAL"),
			ui:Label(t('Wage')):SetFont("HEADING_NORMAL"),
			ui:Label(t('Owed')):SetFont("HEADING_NORMAL"),
			ui:Label(t('Next paid')):SetFont("HEADING_NORMAL"),
		})

		-- Create a row for each crew member
		local wageTotal = 0
		local owedTotal = 0

		for crewMember in Game.player:EachCrewMember() do
			local moreButton = UI.SmallLabeledButton.New(t("More info..."))
			moreButton.button.onClick:Connect(function () return crewMemberInfoButtonFunc(crewMember) end)

			local crewWage = (crewMember.contract and crewMember.contract.wage or 0)
			local crewOwed = (crewMember.contract and crewMember.contract.outstanding or 0)
			wageTotal = wageTotal + crewWage
			owedTotal = owedTotal + crewOwed

			crewlistbox:PackEnd(ui:Grid(rowspec,1):SetRow(0, {
				ui:Label(crewMember.name),
				ui:Label(t(crewMember.title) or t('General crew')),
				ui:Label(Format.Money(crewWage)):SetColor({ r = 0.0, g = 1.0, b = 0.2 }), -- green
				ui:Label(Format.Money(crewOwed)):SetColor({ r = 1.0, g = 0.0, b = 0.0 }), -- red
				ui:Label(Format.Date(crewMember.contract and crewMember.contract.payday or 0)),
				moreButton.widget,
			}))
		end
		crewlistbox:PackEnd(ui:Grid(rowspec,1):SetRow(0, {
			ui:Label(""), -- first column, empty
			ui:Label(t("Total:")):SetFont("HEADING_NORMAL"):SetColor({ r = 1.0, g = 1.0, b = 0.0 }), -- yellow
			ui:Label(Format.Money(wageTotal)):SetColor({ r = 0.0, g = 1.0, b = 0.2 }), -- green
			ui:Label(Format.Money(owedTotal)):SetColor({ r = 1.0, g = 0.0, b = 0.0 }), -- red
		}))

		local taskCrewButton = ui:Button():SetInnerWidget(ui:Label(t('Give orders to crew')))
		taskCrewButton.onClick:Connect(taskCrew)

		return ui:VBox(10):PackEnd({
			headergrid,
			ui:Scroller():SetInnerWidget(crewlistbox),
			taskCrewButton,
		})
	end

	-- Function that creates an info page for a crew member
	-- (local identifier declared earlier)
	crewMemberInfoButtonFunc = function (crewMember)

		-- Make the button that you'd use to sack somebody
		local dismissButton = UI.SmallLabeledButton.New(t("Dismiss"))
		dismissButton.button.onClick:Connect(function ()
			if Game.player.flightState == 'DOCKED' and not(crewMember.contract and crewMember.contract.outstanding > 0) and Game.player:Dismiss(crewMember) then
				crewMember:Save()                         -- Save to persistent characters list
				CrewScreen:SetInnerWidget(makeCrewList()) -- Return to crew roster list
				if crewMember.contract then
					if crewMember.contract.outstanding > 0 then
						Comms.Message(t("I'm tired of working for nothing. Don't you know what a contract is?"),crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 5 -- Hate!
					elseif crewMember:TestRoll('playerRelationship') then
						Comms.Message(t("It's been great working for you. If you need me again, I'll be here a while."),crewMember.name)
					elseif not crewMember:TestRoll('lawfulness') then
						Comms.Message(t("You're going to regret sacking me!"),crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 1
					else
						Comms.Message(t("Good riddance to you, too."),crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 1
					end
				end
			end
		end)

		CrewScreen:SetInnerWidget(ui:Grid(2,1)
		-- Set left hand side of page: General information about the Character
		:SetColumn(0, {
			ui:VBox(20):PackEnd({
				ui:Label(crewMember.name):SetFont("HEADING_LARGE"),
				ui:Label(t("Qualification scores")):SetFont("HEADING_NORMAL"),
				-- Table of crew scores:
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(t("Engineering:")),
							ui:Label(t("Piloting:")),
							ui:Label(t("Navigation:")),
							ui:Label(t("Sensors:")),
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							ui:Label(crewMember.engineering),
							ui:Label(crewMember.piloting),
							ui:Label(crewMember.navigation),
							ui:Label(crewMember.sensors),
						})
					}),
				-- Things we can do with this crew member
				--  (as long as they're not the player!)
				-- returning nil if crewMember is player
				not crewMember.player and ui:Label(t("Employment")):SetFont("HEADING_NORMAL") or nil,
				not crewMember.player and ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							dismissButton,
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							UI.SmallLabeledButton.New(t("Negotiate")),
						})
					}) or nil -- nothing returned for player
			})
		})
		-- Set Right hand side of page: Character's face
		:SetColumn(1, { UI.InfoFace.New(crewMember) }))
	end

	CrewScreen:SetInnerWidget(makeCrewList())

	return CrewScreen
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
	tabGroup:AddTab({ id = "crew",            title = t("Crew Roster"),          icon = "Agenda",    template = crewRoster,      })
	--tabGroup:AddTab({ id = "orbitalAnalysis", title = t("Orbital Analysis"),     icon = "Planet",    template = orbitalAnalysis, })

	return tabGroup.widget
end

