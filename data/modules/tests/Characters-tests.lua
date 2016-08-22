local loaded

local onGameStart = function ()

	if not (loaded == nil) then
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
	assert (test > bob.luck,"Character assertion failed: TestRoll() didn't punish a critical failure")
	test = charlie.luck
	assert(charlie:TestRoll('luck'),"Character assertion failed: TestRoll() didn't pass on maximum roll")
	assert (test < charlie.luck,"Character assertion failed: TestRoll() didn't reward a critical failure")
	test = derek.luck
	derek:TestRoll()
	assert (test == derek.luck,"Character assertion failed: TestRoll() treated median roll as critical")
	print('TestRoll() passed tests')

	--SafeRoll()
	assert(type(alice.SafeRoll) == 'function',"Character assertion failed: SafeRoll not a function")
	assert(type(alice:SafeRoll()=='boolean'),"Character assertion failed: SafeRoll() didn't return boolean")
	test = bob.luck
	bob:SafeRoll()
	assert(not bob:SafeRoll('luck'),"Character assertion failed: SafeRoll() didn't fail on minimum roll")
	assert (test == bob.luck,"Character assertion failed: SafeRoll() modified an attribute")
	test = charlie.luck
	charlie:SafeRoll()
	assert(charlie:SafeRoll('luck'),"Character assertion failed: SafeRoll() didn't pass on maximum roll")
	assert (test == charlie.luck,"Character assertion failed: SafeRoll() modified an attribute")
	print('SafeRoll() passed tests')

	--FindAvailable()
	assert(type(Character.FindAvailable) == 'function',"Character assertion failed: FindAvailable not a function")
	assert(type(Character.FindAvailable()=='function'),"Character assertion failed: FindAvailable() didn't return a function")
	test=0
	for v in Character.FindAvailable() do
		test = test + 1
		assert(v.available,"Character assertion failure: FindAvailable() returned an unavailable character")
	end
	assert(test==2,"Character assertion failed: FindAvailable() returned wrong number of characters") --bob,charlie
	print('FindAvailable() passed tests')

	--Find()
	assert(type(Character.Find) == 'function',"Character assertion failed: Find not a function")
	assert(type(Character.Find()=='function'),"Character assertion failed: Find() didn't return a function")
	test=0
	for v in Character.Find() do
		test = test + 1
	end
	assert(test==3,"Character assertion failed: Find() returned wrong number of characters with no filter") --alice,bob,charlie
	test=0
	for v in Character.Find(function (t) return t.name == derek.name end) do
		assert(test==0,"Character assertion failed: Find() returned wrong number of characters with filter") --alice
		assert(v.name==derek.name,"Character assertion failed: Find() returned wrong character using filter") --derek is a Clone() of alice
		test = test + 1
	end
	print('Find() passed tests')

	--UnSave()
	assert(type(alice.UnSave) == 'function',"Character assertion failed: UnSave not a function")
	assert(type(alice:UnSave()=='number'),"Character assertion failed: UnSave() didn't return a number")
	alice:UnSave()
	assert(#PersistentCharacters==2,"Character assertion failed: unSave() didn't remove a character")
	for v in Character.Find() do
		assert(v~=alice,"Character assertion failed: UnSave() removed the wrong character")
	end
	print('UnSave() passed tests')

	--RollNew()
	assert(type(alice.RollNew) == 'function',"Character assertion failed: RollNew not a function")
	derek = Character.New()
	test = 0
	local base = 0
	for k,v in pairs(derek) do
		test = test + 1
	end
	base = test
	derek:RollNew()
	test = 0
	for k,v in pairs(derek) do
		test = test + 1
	end
	assert(test == base + 4,"Character assertion failure: RollNew() initialized wrong number of attributes")
	derek:RollNew(true)
	test = 0
	for k,v in pairs(derek) do
		test = test + 1
	end
	assert(test == base + 8,"Character assertion failure: RollNew(true) initialized wrong number of attributes")
	print('RollNew() passed tests')

	alice:UnSave()
	bob:UnSave()
	charlie:UnSave()

end

local serialize = function ()
	return { something = {} }
end

local unserialize = function (data)
	loaded = data
end

EventQueue.onGameStart:Connect(onGameStart)
Serializer:Register("Characters-tests", serialize, unserialize)
