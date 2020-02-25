-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local util = {}

function util.mixin_modules(t)
    t.modules = {}
    t.moduleCount = 0

    function t.registerModule(name, module)
        t.modules[name] = module
        table.insert(t.modules, module)
        t.moduleCount = t.moduleCount + 1
    end
end

return util
