local SheetDisplayData = function (sheet)
	return ([[{name}
--ATTRIBUTES--
Luck: {luck}
Charisma: {charisma}
Notoriety: {notoriety}
Lawfulness: {lawfulness}
--SKILLS--
Engineering: {engineering}
Piloting: {piloting}
Navigation: {navigation}
Sensors: {sensors}
--DATA--
Use count: {useCount}, Last seen: {time}, Location: {place}]]):interp({
		name = sheet.name,
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

local onChat = function (form, ref, option)
	if option == 0 then
		form:Clear()

		form:SetTitle('Character sheet testing')
		form:SetFace(PersistentCharacters.player)
		if not PersistentCharacters[1] then (Character.New()):Save() end
		form:SetMessage(SheetDisplayData(PersistentCharacters[1]))

		form:AddOption("Create a new character", 1)

		return
	end

	if option == 1 then
	end

end

local onCreateBB = function (station)
	station:AddAdvert('CHARACTER SHEET TESTING', onChat, onDelete)
end

EventQueue.onCreateBB:Connect(onCreateBB)
