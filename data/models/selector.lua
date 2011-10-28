function reg_no(self)
regn = get_label()
end

function preselect(self)   -- generates a "random" number as replacement for symbol characters (-,?,!,+, etc)
reg_no()
math.randomseed(tonumber(string.sub((string.gsub(regn,"%W", 1)),1,3),36))
presel = math.random(1,10)
--print ("pre")
--print (presel)
end

function selector1(self)  -- decal and squad selection
reg_no()
preselect()
local seed1 = math.randomseed(tonumber(string.sub((string.gsub(regn,"%W", presel)),4,7),36)) --[[ generates a number from ship registration for random seed, any string is converted to a number,
																					  with replacement of non-alphanumerical by 1, because you might like to "hack" your savegame 
																					  to give your ship a name instead of a reg#, e.g. "KILLER!" or " LUCKY-7", then decal selection still works!
																					  further modelviewers given reg# "IR-L33T" still works with that to ;)
																					  the selection method works now also for variable skins and materials which should depend on ships reg.
																					  this allows to have the classic squadron colors, red, gold, blue & green 8)
	    																			  --]]
select1 = math.random(1,1000) -- randomizes 1 to 1000, uses seed for a uniqe number in between 1 to 1000 (inclusive), identical numbers produce the same result
--print  ("sel1")
--print (select1)  -- use this to check random result
end

function selector2(self) -- skin color selection
preselect()
reg_no()
math.randomseed(tonumber(string.sub((string.gsub(regn,"%W", presel)),3,6),36))
select2 = math.random(1,100)
--print ("sel2")
--print (select2)
end

function selector3(self) -- dress color selection
preselect()
reg_no()
math.randomseed(tonumber(string.sub((string.gsub(regn,"%W", presel)),2,5),36))
select3 = math.random(1,100)
--print ("sel3")
--print (select3)
end

function selector4(self) -- female / male selection
preselect()
reg_no()
math.randomseed(tonumber(string.sub((string.gsub(regn,"%W", presel)),1,4),36))
select4 = math.random(1,100)
--print ("sel4")
--print (select4)
end


