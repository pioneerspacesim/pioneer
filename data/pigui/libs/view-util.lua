-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local util = {}

function util.mixin_modules(t)
	t.modules = {}
	t.moduleCount = 0

	function t.registerModule(name, module)
		-- replace if such name already exists
		if t.modules[name] then
			t.modules[t.modules[name]] = module
		else
			table.insert(t.modules, module)
			t.moduleCount = t.moduleCount + 1
			t.modules[name] = t.moduleCount
		end
	end
end

return util
