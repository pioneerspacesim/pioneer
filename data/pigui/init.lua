-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


-- for some reason, this is required or the functions on Game.Player
-- aren't available later...
local Player = require 'Player'

local ui = require 'pigui.baseui'

require 'pigui.libs.text'
require 'pigui.libs.icons'
require 'pigui.libs.buttons'
require 'pigui.libs.radial-menu'
require 'pigui.libs.gauge'


return ui
