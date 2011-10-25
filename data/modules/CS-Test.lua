local SheetDisplayData = function (sheet)
	return ([[ Luck: {luck}
Charisma: {charisma}
Notoriety: {notoriety}
Lawfulness: {lawfulness}
--
Engineering: {engineering}
Piloting: {piloting}
Navigation: {navigation}
Sensors: {sensors}
--
Use count: {useCount}, Last seen: {time}, Location: {place}]]):interp({
		luck = sheet.luck,
		charisma = sheet.charisma,
		notoriety = sheet.notoriety,
		lawfulness = sheet.lawfulness,
		engineering = sheet.engineering,
		piloting = sheet.piloting,
		navigation = sheet.navigation,
		sensors = sheet.sensors,
		useCount = sheet.useCount,
		time = Format.Date(sheet.lastSavedTime),
		place = ((sheet.lastSavedSystemPath):GetSystemBody()).name,
	})
end

local current

local onChat = function (form, ref, option)

	local mainmenu = function ()
		form:Clear()
		form:SetTitle('Character sheet testing')
		form:SetFace(PersistentCharacters.player)
		form:SetMessage('First ten persistent characters are listed below')

		form:AddOption("Create a new persistent character", 1)
		for topten = 1,10 do
			if PersistentCharacters[topten] then
				form:AddOption(PersistentCharacters[topten].name,topten+1)
			end
		end
		return
	end

	local showsheet = function(sheet)
		form:Clear()
		form:SetTitle("CHARACTER SHEET: " .. sheet.name)
		form:SetFace(sheet)
		form:SetMessage(SheetDisplayData(sheet))
		form:AddOption('Randomize',13)
		form:AddOption('Randomize for crew',14)
		form:AddOption('Make non-persistent',15)
		form:AddOption('Test this character',16)
		form:AddOption('Done',0)
	end

	if option == 0 then
		mainmenu()
	end

	if option == 1 then
		(Character.New()):Save()
		mainmenu()
	end

	if option > 1 and option < 12 then
		current = PersistentCharacters[option - 1]
		showsheet(current)
	end

	if option == 13 then
		current:RollNew()
		showsheet(current)
	end

	if option == 14 then
		current:RollNew(true)
		showsheet(current)
	end

	if option == 15 then
		current:UnSave()
		showsheet(current)
	end

	if option == 16 then
		form:Clear()
		form:SetTitle("TESTING: " .. current.name)
		form:SetFace(current)
		form:SetMessage([[RESULTS
Luck:
Charisma:
Notoriety:
Lawfulness:
--
Engineering:
Piloting:
Navigation:
Sensors:
]])
		form:AddOption('Test all attributes',18)
		form:AddOption('Back to character sheet',17)
		form:AddOption('Back to character list',0)
	end

	if option == 17 then
		showsheet(current)
	end

	local passfail = function (test)
		if test then return 'Pass' else return 'Fail' end
	end

	if option == 18 then
		form:SetMessage(([[RESULTS
Luck: {luck}
Charisma: {charisma}
Notoriety: {notoriety}
Lawfulness: {lawfulness}
--
Engineering: {engineering}
Piloting: {piloting}
Navigation: {navigation}
Sensors: {sensors}
]]):interp({
			luck = passfail(current:TestRoll('luck')),
			charisma = passfail(current:TestRoll('charisma')),
			notoriety = passfail(current:TestRoll('notoriety')),
			lawfulness = passfail(current:TestRoll('lawfulness')),
			engineering = passfail(current:TestRoll('engineering')),
			piloting = passfail(current:TestRoll('piloting')),
			navigation = passfail(current:TestRoll('navigation')),
			sensors = passfail(current:TestRoll('sensors')),
		}))
	end

end

local onCreateBB = function (station)
	station:AddAdvert('CHARACTER SHEET TESTING', onChat, onDelete)
end

EventQueue.onCreateBB:Connect(onCreateBB)
