-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Format = import("Format")
local ShipDef = import("ShipDef")
local Comms = import("Comms")
local Equipment = import("Equipment")


local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

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
			ATTEMPT_TO_REPAIR_HULL = function ()
				-- Convoluted...
				local hullMass = ShipDef[Game.player.shipId].hullMass
				local hullMassLeft = Game.player.hullMassLeft
				local hullDamage = hullMass - hullMassLeft
				if hullDamage > 0 then
					if Game.player:CountEquip(Equipment.cargo.metal_alloys, cargo) <= 0 then
						feedback:SetText(l.NOT_ENOUGH_ALLOY_TO_ATTEMPT_A_REPAIR:interp({alloy = l.METAL_ALLOYS}))
						return
					end
					local crewMember, result = testCrewMember('engineering',true)
					if crewMember then
						local repair = math.min(
							-- Need metal alloys for repair. Check amount.
							math.ceil(hullDamage/(64 - result)), -- 65 > result > 3
							Game.player:CountEquip(Equipment.cargo.metal_alloys, cargo)
						)
						Game.player:RemoveEquip(Equipment.cargo.metal_alloys, repair) -- These will now be part of the hull.
						local repairPercent = math.min(math.ceil(100 * (repair + hullMassLeft) / hullMass), 100) -- Get new hull percentage...
						Game.player:SetHullPercent(repairPercent)   -- ...and set it.
						feedback:SetText(l.HULL_REPAIRED_BY_NAME_NOW_AT_N_PERCENT:interp({name = crewMember.name,repairPercent = repairPercent}))
					else
						local repairPercent = math.max(math.floor(100 * (hullMassLeft - 1) / hullMass), 1) -- Get new hull percentage...
						Game.player:SetHullPercent(repairPercent)   -- ...and set it.
						feedback:SetText(l.HULL_REPAIR_ATTEMPT_FAILED_HULL_SUFFERED_MINOR_DAMAGE)
					end
				else
					feedback:SetText(l.HULL_DOES_NOT_REQUIRE_REPAIR)
				end
			end,

			DESTROY_ENEMY_SHIP = function ()
				if Game.player.flightState ~= 'FLYING'
				then
					feedback:SetText(({
						DOCKED = l.YOU_MUST_REQUEST_LAUNCH_CLEARANCE_FIRST_COMMANDER,
						LANDED = l.YOU_MUST_LAUNCH_FIRST_COMMANDER,
						JUMPING = l.WE_ARE_IN_HYPERSPACE_COMMANDER,
						HYPERSPACE = l.WE_ARE_IN_HYPERSPACE_COMMANDER,
						DOCKING = l.THE_SHIP_IS_UNDER_STATION_CONTROL_COMMANDER,
						UNDOCKING = l.THE_SHIP_IS_UNDER_STATION_CONTROL_COMMANDER,
					})[Game.player.flightState])
				elseif not Game.player:GetCombatTarget() then
					feedback:SetText(l.YOU_MUST_FIRST_SELECT_A_COMBAT_TARGET_COMMANDER)
				else
					local crewMember = checkPilotLockout() and testCrewMember('piloting')
					if not crewMember then
						feedback:SetText(l.THERE_IS_NOBODY_ELSE_ON_BOARD_ABLE_TO_FLY_THIS_SHIP)
						pilotLockout()
					else
						feedback:SetText(l.PILOT_SEAT_IS_NOW_OCCUPIED_BY_NAME:interp({name = crewMember.name}))
						Game.player:AIKill(Game.player:GetCombatTarget())
					end
				end
			end,

			DOCK_AT_CURRENT_TARGET = function ()
				local target = Game.player:GetNavTarget()
				if Game.player.flightState ~= 'FLYING'
				then
					feedback:SetText(({
						DOCKED = l.YOU_MUST_REQUEST_LAUNCH_CLEARANCE_FIRST_COMMANDER,
						LANDED = l.YOU_MUST_LAUNCH_FIRST_COMMANDER,
						JUMPING = l.WE_ARE_IN_HYPERSPACE_COMMANDER,
						HYPERSPACE = l.WE_ARE_IN_HYPERSPACE_COMMANDER,
						DOCKING = l.THE_SHIP_IS_UNDER_STATION_CONTROL_COMMANDER,
						UNDOCKING = l.THE_SHIP_IS_UNDER_STATION_CONTROL_COMMANDER
					})[Game.player.flightState])
				elseif not (target and target:isa('SpaceStation')) then
					feedback:SetText(l.YOU_MUST_FIRST_SELECT_A_SUITABLE_NAVIGATION_TARGET_COMMANDER)
				else
					local crewMember = checkPilotLockout() and testCrewMember('piloting')
					if not crewMember then
						feedback:SetText(l.THERE_IS_NOBODY_ELSE_ON_BOARD_ABLE_TO_FLY_THIS_SHIP)
						pilotLockout()
					else
						feedback:SetText(l.PILOT_SEAT_IS_NOW_OCCUPIED_BY_NAME:interp({name = crewMember.name}))
						Game.player:AIDockWith(target)
					end
				end
			end,
		}

		local taskList = ui:VBox() -- This could do with being something prettier

		for label,task in pairs(crewTasks) do
			local taskButton = SmallLabeledButton.New(l[label])
			taskButton.button.onClick:Connect(task)
			taskList:PackEnd(taskButton)
		end
		taskList:PackEnd(feedback)

		CrewScreen:SetInnerWidget(taskList)
	end

	-- Function that creates the crew list
	local makeCrewList = function ()
		local crewTable =
			ui:Table()
				:SetHeadingRow({l.NAME_PERSON, l.POSITION, l.WAGE, l.OWED, l.NEXT_PAID})
				:SetHeadingFont("HEADING_NORMAL")
				:SetRowSpacing(5)
				:SetRowAlignment("CENTER")
				:SetColumnAlignment("JUSTIFY")

		-- Create a row for each crew member
		local wageTotal = 0
		local owedTotal = 0

		for crewMember in Game.player:EachCrewMember() do
			local moreButton = SmallLabeledButton.New(l.MORE_INFO)
			moreButton.button.onClick:Connect(function () return crewMemberInfoButtonFunc(crewMember) end)

			local crewWage = (crewMember.contract and crewMember.contract.wage or 0)
			local crewOwed = (crewMember.contract and crewMember.contract.outstanding or 0)
			wageTotal = wageTotal + crewWage
			owedTotal = owedTotal + crewOwed

			crewTable:AddRow({
				crewMember.name,
				crewMember.title or l.GENERAL_CREW,
				ui:Label(Format.Money(crewWage)):SetColor({ r = 0.0, g = 1.0, b = 0.2 }), -- green
				ui:Label(Format.Money(crewOwed)):SetColor({ r = 1.0, g = 0.0, b = 0.0 }), -- red
				Format.Date(crewMember.contract and crewMember.contract.payday or 0),
				moreButton
			})
		end
		crewTable:AddRow({
			"", -- first column, empty
			ui:Label(l.TOTAL):SetFont("HEADING_NORMAL"):SetColor({ r = 1.0, g = 1.0, b = 0.0 }), -- yellow
			ui:Label(Format.Money(wageTotal)):SetColor({ r = 0.0, g = 1.0, b = 0.2 }), -- green
			ui:Label(Format.Money(owedTotal)):SetColor({ r = 1.0, g = 0.0, b = 0.0 }), -- red
		})

		local taskCrewButton = ui:Button():SetInnerWidget(ui:Label(l.GIVE_ORDERS_TO_CREW))
		taskCrewButton.onClick:Connect(taskCrew)

		return ui:VBox(10):PackEnd({
			crewTable,
			taskCrewButton,
		})
	end

	-- Function that creates an info page for a crew member
	-- (local identifier declared earlier)
	crewMemberInfoButtonFunc = function (crewMember)

		-- Make the button that you'd use to sack somebody
		local dismissButton = SmallLabeledButton.New(l.DISMISS)
		dismissButton.button.onClick:Connect(function ()
			if Game.player.flightState == 'DOCKED' and not(crewMember.contract and crewMember.contract.outstanding > 0) and Game.player:Dismiss(crewMember) then
				crewMember:Save()                         -- Save to persistent characters list
				CrewScreen:SetInnerWidget(makeCrewList()) -- Return to crew roster list
				if crewMember.contract then
					if crewMember.contract.outstanding > 0 then
						Comms.Message(l.IM_TIRED_OF_WORKING_FOR_NOTHING_DONT_YOU_KNOW_WHAT_A_CONTRACT_IS,crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 5 -- Hate!
					elseif crewMember:TestRoll('playerRelationship') then
						Comms.Message(l.ITS_BEEN_GREAT_WORKING_FOR_YOU_IF_YOU_NEED_ME_AGAIN_ILL_BE_HERE_A_WHILE,crewMember.name)
					elseif not crewMember:TestRoll('lawfulness') then
						Comms.Message(l.YOURE_GOING_TO_REGRET_SACKING_ME,crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 1
					else
						Comms.Message(l.GOOD_RIDDANCE_TO_YOU_TOO,crewMember.name)
						crewMember.playerRelationship = crewMember.playerRelationship - 1
					end
				end
			end
		end)

		CrewScreen:SetInnerWidget(ui:Grid({48,4,48},1)
		-- Set left hand side of page: General information about the Character
		:SetColumn(0, {
			ui:VBox(20):PackEnd({
				ui:Label(crewMember.name):SetFont("HEADING_LARGE"),
				ui:Label(l.QUALIFICATION_SCORES):SetFont("HEADING_NORMAL"),
				-- Table of crew scores:
				ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							ui:Label(l.ENGINEERING),
							ui:Label(l.PILOTING),
							ui:Label(l.NAVIGATION),
							ui:Label(l.SENSORS),
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
				not crewMember.player and ui:Label(l.EMPLOYMENT):SetFont("HEADING_NORMAL") or nil,
				not crewMember.player and ui:Grid(2,1)
					:SetColumn(0, {
						ui:VBox():PackEnd({
							dismissButton,
						})
					})
					:SetColumn(1, {
						ui:VBox():PackEnd({
							SmallLabeledButton.New(l.NEGOTIATE),
						})
					}) or nil -- nothing returned for player
			})
		})
		-- Set Right hand side of page: Character's face
		:SetColumn(2, { InfoFace.New(crewMember) }))
	end

	CrewScreen:SetInnerWidget(makeCrewList())

	return CrewScreen
end

return crewRoster
