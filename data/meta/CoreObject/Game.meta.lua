-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis
-- TODO: this file is partially type-complete, please expand it as more types are added.

---@meta

---@class Game
---
---@field player Player The player's current ship
---@field system StarSystem? The system the game is currently playing in or nil when in hyperspace.
---@field systemView unknown #TODO: add type info for SystemView interface
---@field sectorView unknown #TODO: add type info for SectorView interface
---@field time number Game time in seconds since Jan. 1 3200
---@field paused boolean

---@class Game
local Game = {}

-- Ensure the CoreImport field is visible to static analysis
package.core["Game"] = Game

--- Start a game at the given location and game world time.
---
--- If the given path is a starport, will spawn docked at that starport;
--- otherwise the player will spawn in orbit around the specified body.
---@param path SystemPath path to a body to start at
---@param start_time number optional, default to real time since 31 Dec. 1999
function Game.StartGame(path, start_time) end

--- Attempts to load and start the specified savefile.
--- Will throw an error if the game could not be loaded.
---@param filename string
function Game.LoadGame(filename) end

--- Try to load the given savefile and determine if it is in a valid format.
---@param filename string
---@return boolean
function Game.CanLoadGame(filename) end

--- Get stats about the specified save game
---@param filename string
---@return table stats
function Game.SaveGameStats(filename) end

--- Save the current game to the given savefile.
---@param filename string
function Game.SaveGame(filename) end

--- Delete savefile with specified filename.
---@param filename string
function Game.DeleteSave(filename) end

--- End the current game and return to the main menu.
function Game.EndGame() end

--- Switch view back to world view if player is not dead.
---@deprecated
function Game.SwitchView() end

---@return string
function Game.CurrentView() end

---@param view string
function Game.SetView(view) end

--- Returns the current game time split into y/m/d/h/m/s components
---@return integer year
---@return integer month
---@return integer day
---@return integer hour
---@return integer minute
---@return integer second
function Game.GetDateTime() end

--- Returns the given time in seconds since 1 Jan. 3200
-- split into y/m/d/h/m/s components
---@param time number
---@return integer year
---@return integer month
---@return integer day
---@return integer hour
---@return integer minute
---@return integer second
function Game.GetPartsFromDateTime(time) end

---@param newAccel string
---@param force boolean?
function Game.SetTimeAcceleration(newAccel, force) end
---@return string
function Game.GetTimeAcceleration() end
---@return string
function Game.GetRequestedTimeAcceleration() end

---@return boolean
function Game.InHyperspace() end
---@return number
function Game.GetHyperspaceTravelledPercentage() end

---@return string
function Game.GetWorldCamType() end
---@param type string
function Game.SetWorldCamType(type) end

return Game
