local onGameStart = function ()

	if new_start == false then
		print('Skipping Character class tests on loaded game')
		return
	end

	-- Our cast.
	print('Testing Character class')
	local alice = Character.New()
	local bob = Character.New()
	local charlie = Character.New()

	-- New()
	assert(not alice.player,"New characters shouldn't be players")
	for i,attr in ipairs ({'luck','charisma','notoriety','lawfulness',
	                      'engineering','piloting','navigation','sensors'}) do
		alice[attr]=nil
		assert(type(alice[attr]) == 'number',("Character assertion failed:{attr} not inherited as number"):interp({attr=attr}))
	end
	print('New() passed tests')

	-- Save()
	assert(type(alice.Save) == 'function',"Character assertion failed: Save not a function")
	alice.available=false
	PCcount = #PersistentCharacters
	spot = alice:Save()
	assert(PersistentCharacters[spot] == alice,'Character assertion failed: Save() failed to return index of saved character')
	assert(#PersistentCharacters == PCcount+1,'Character assertion failed: Save didn failed to increase size of persistent characters array')
	assert(alice.available,'Save failed to mark character as available')
	PCcount = #PersistentCharacters
	spot = bob:Save()
	assert(PersistentCharacters[spot] == bob,'Character assertion failed: Save() failed to return index of saved character')
	assert(#PersistentCharacters == PCcount+1,'Character assertion failed: Save() failed to increase size of persistent characters array')
	PCcount = #PersistentCharacters
	spot = charlie:Save()
	assert(PersistentCharacters[spot] == charlie,'Character assertion failed: Save() failed to return index of saved character')
	assert(#PersistentCharacters == PCcount+1,'Character assertion failed: Save didn failed to increase size of persistent characters array')
	print('Save() passed tests')

	--CheckOut()
	assert(type(alice.CheckOut) == 'function',"Character assertion failed: CheckOut not a function")
	alice:CheckOut()
	assert(not alice.available,"CheckOut() failed to mark character as unavailable")
	print('CheckOut() passed tests')

	--Clone()
	assert(type(alice.Clone) == 'function',"Character assertion failed: Clone not a function")
	alice.luck = 1 --arbitrary
	alice.charisma = nil --force inherit
	local derek = alice:Clone()
	assert(type(derek)=='table',"Clone() failed to copy Character class")
	local test = getmetatable(derek)
	assert(test==Character.meta,"Clone() failed to set inheritance")
	assert(derek.name == alice.name,"Clone() failed to copy name")
	assert(derek.luck == alice.luck,"Clone() failed to copy set value")
	assert(derek.charisma == alice.charisma,"Clone() failed to set inheritance")
	assert(derek.Relationships ~= alice.Relationships,"Clone() failed to initialise new Relationships table")
	print('Clone() passed tests')

	--Make some predictable dice rolls by overloading ; assumes 4d16
	assert(type(alice.DiceRoll) == 'function',"Character assertion failed: DiceRoll not a function")
	assert(type(alice.DiceRoll()=='number'),"Character assertion failed: DiceRoll() didn't return a number")
	print('DiceRoll() passed tests')
	bob.DiceRoll = function () return 64 end --max
	charlie.DiceRoll = function () return 4 end --min
	derek.DiceRoll = function () return 34 end --median

	--TestRoll()
	assert(type(alice.TestRoll) == 'function',"Character assertion failed: TestRoll not a function")
	assert(type(alice:TestRoll()=='boolean'),"Character assertion failed: TestRoll() didn't return boolean")
	test = bob.luck
	assert(not bob:TestRoll('luck'),"Character assertion failed: TestRoll() didn't fail on minimum roll")
	print(test,bob.luck)
	assert (test > bob.luck,"Character assertion failed: TestRoll() didn't punish a critical failure")
	test = charlie.luck
	assert(charlie:TestRoll('luck'),"Character assertion failed: TestRoll() didn't pass on maximum roll")
	assert (test < charlie.luck,"Character assertion failed: TestRoll() didn't reward a critical failure")
	test = derek.luck
	derek:TestRoll()
	assert (test == derek.luck,"Character assertion failed: TestRoll() treated median roll as critical")
	print('TestRoll() passed tests')

end

local new_start = true

local serialize = function ()
	return { loaded = true }
end

local unserialize = function (data)
	if data.loaded then new_start = false end
end

EventQueue.onGameStart:Connect(onGameStart)
Serializer:Register("Characters-tests", serialize, unserialize)
