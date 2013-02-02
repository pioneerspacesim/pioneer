-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

math.clamp = function(v, min, max)
	return math.min(max, math.max(v,min))
end

debug.deprecated = function()
	local deprecated_function = debug.getinfo(2)
	local caller = debug.getinfo(3)
	print("The use of the function \""..deprecated_function.name.."\" as done at <"..caller.source..":"..caller.currentline.."> is deprecated")
	print("Please check the changelogs and/or get in touch with the development team.")
end


--
-- Various table functions
-- Adapted from: https://bitbucket.org/luafunctional/lua-functional/src
--

-- filter
-- Take out all elements of a list not satisfying some condition.
-- Parameters:
--   func: a function applied to each element and returning true or false
--   t:    a table (the list)
--   iter: (optional) an iterator to use with t
--     Default value: ipairs
-- Return:
--   t2: a new table satisying that, for each k,v returned by iter,
--       if func(v) then add v to t2
table.filter = function(func, t, iter)
	iter = iter or ipairs
	local t2 = {}

	for k,v in iter(t) do
		if func(v) then table.insert(t2,v) end
	end

	return t2
end

-- map
-- Apply a function to each element of an array or table.
-- Return a new table satisfying that, for each k,v returned
-- by 'iter', t2[k] = func(v)
-- Parameters:
--   func: the function to apply
--   t: the table we apply 'func' to
--   iter: (optional) an iterator over t
--     Default value: pairs
-- Return:
--   t2: the new table
table.map = function(func, t, iter)
	iter = iter or pairs
	local t2 = {}

	for k,v in iter(t) do
		t2[k] = func(v)
	end

	return t2
end
