-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local util = {}

function util.mixin_modules(modules, callback)
	---@param name string
	---@param module table
	return function (name, module)
		local idx = modules[name]
		-- replace if such name already exists
		if idx then
			modules[idx] = module
		else
			table.insert(modules, module)
			modules[name] = #modules
		end

		if callback then callback(name, module, modules[name]) end
	end
end

return util
