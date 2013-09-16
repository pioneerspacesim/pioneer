-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")
local Game = import("Game")
local Format = import("Format")
local ShipType = import("ShipType")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local t = Translate:GetTranslator()

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
			local taskButton = SmallLabeledButton.New(t(label))
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
			local moreButton = SmallLabeledButton.New(t("More info..."))
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
		local dismissButton = SmallLabeledButton.New(t("Dismiss"))
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
							SmallLabeledButton.New(t("Negotiate")),
						})
					}) or nil -- nothing returned for player
			})
		})
		-- Set Right hand side of page: Character's face
		:SetColumn(1, { InfoFace.New(crewMember) }))
	end

	CrewScreen:SetInnerWidget(makeCrewList())

	return CrewScreen
end

return crewRoster
