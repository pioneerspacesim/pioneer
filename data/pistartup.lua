-- Pioneer Lua startup

-- XXX for the moment the individual start scripts call this if they need it
--     (mostly if they're using PiLuaClasses). once everything is the same
--     across the codebase and we have a single place in the engine to
--     instantiate a lua context this will be loaded automatically.

-- rename wrapper classes
fixed  = pi_fixed
vector = pi_vector

-- XXX Console is available in the main game Lua environment,
--     but pistartup.lua is also used from CustomSystems which doesn't have Console
if Console ~= nil then
	local builtin_print = print
	print = function (...)
		-- emit to the console
		local t = {...}
		-- note: can't use pairs() or ipairs() here,
		-- because they skip nil values
		for i = 1,#t do
			t[i] = tostring(t[i])
		end
		Console.AddLine(table.concat(t, '\t'))

		-- forward to the builtin print as well
		builtin_print(...)
	end
end

-- convenience constructors
-- XXX do these belong in eg picompat.lua?
function v (...) return vector:new(...) end
function norm (...) return vector:new(...):norm() end
function f (...) return fixed:new(...) end

-- math library extensions
function math.deg2rad (n) return n * math.pi / 180 end

-- XXX I think I want a fixed math lib. if I do make one it'll probably be
--     named like this
math.fixed = {}
function math.fixed.deg2rad (n) return n * fixed:new(31416,1800000) end

-- a nice string interpolator
string.interp = function (s, t)
	return (s:gsub('(%b{})', function(w) return t[w:sub(2,-2)] or w end))
end
 
