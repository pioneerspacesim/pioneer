-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipDef = require 'ShipDef'
local debugView = require 'pigui.views.debug'
local Engine = require 'Engine'
local e = require 'Equipment'
local Game = require 'Game'
local ui = require 'pigui.baseui'
local utils = require 'utils'
local CommodityType = require 'CommodityType'
local Vector2 = Vector2
local Space = require 'Space'

local Core = require 'modules.TradeShips.Core'

local arrayTable = require 'pigui.libs.array-table'

-- this module creates a tab in the debug window

-- UTILS

-- create a class method caller
local function method(fnc, ...)
	local args = {...}
	return function(x)
		return x[fnc](x, table.unpack(args))
	end
end

-- create a formatting function
local function format(fmt)
	return function(x)
		return string.format(fmt, x)
	end
end

-- convert lenght in meters to string in AU
local function distanceInAU(meters)
	return string.format("%.2f AU", meters / Core.AU)
end

local search_text = ""
local infosize = 0
local statuses = {
	'inbound',
	'docked',
	'hyperspace',
	'hyperspace_out',
	'orbit',
	'fleeing',
	'outbound',
	'cowering',
	'unknown'
}

debugView.registerTab('debug-trade-ships', function()
	if not Core.ships and not Core.params or not Game.system then return end
	if not ui.beginTabItem("Tradeships") then return end

	local function property(key, value)
		ui.withStyleColors({["Text"] = ui.theme.colors.blueFrame}, function()
			ui.text(key)
		end)
		ui.sameLine()
		ui.text(value)
	end

	ui.child("tradeships_as_child", Vector2(-1, -infosize), {"NoSavedSettings", "HorizontalScrollbar"}, function()
		if Core.params then
		if ui.collapsingHeader("System summary", {"DefaultOpen"}) then
			local precalculated = {}
			if Core.params.spawn_in then
				for _, param in ipairs(Core.params.spawn_in) do
					precalculated[param[1]] = param[2]
				end
			end
			local number_of = {}
			for _, status in ipairs(statuses) do
				number_of[status] = 0
			end
			local total_ships = 0
			for _, trader in pairs(Core.ships) do
				if number_of[trader.status] then
					number_of[trader.status] = number_of[trader.status] + 1
				else
					number_of['unknown'] = number_of['unknown'] + 1
				end
				total_ships = total_ships + 1
			end

			arrayTable.draw("tradeships_statuses", statuses, arrayTable.addKeys(ipairs, {
				ships_fact = function(_,v) return number_of[v] end,
				ships_proj = function(_,v) return precalculated[v] end,
				ratio      = function(_,v) return precalculated[v] and number_of[v] / precalculated[v] * 100 end
			}),{
				{ name = "Status",     key = 1,            string = true          },
				{ name = "Current",    key = "ships_fact",                        },
				{ name = "Calculated", key = "ships_proj", fnc = format("%.2f")   },
				{ name = "%",          key = "ratio",      fnc = format("%.2f%%") }
			},{
				totals = {{status = "Total", ships_fact = total_ships}}
			})
			ui.separator()
			property("Total flow", string.format("%.2f ship/hour", Core.params.total_flow))
			ui.sameLine()
			property("Last spawn interval", ui.Format.Duration(Core.last_spawn_interval))
			ui.sameLine()
			property("Lawlessness", string.format("%.4f", Game.system.lawlessness))
			ui.sameLine()
			property("Total bodies in space", Space.GetNumBodies())
		end

		if ui.collapsingHeader("Stations") then
			local totals = {docks = 0, busy_s = 0, landed = 0, flow = 0}
			-- count the inbound ships
			local inbound = {}
			for _, trader in pairs(Core.ships) do
				local s = trader.status == 'inbound' and trader.starport
				if s then
					if inbound[s] then inbound[s] = inbound[s] + 1
					else inbound[s] = 1
					end
				end
			end
			totals.label = "Total for " .. utils.count(Core.params.port_params) .. " ports"
			local obj = Game.systemView:GetSelectedObject()
			local sb_selected = obj.type == Engine.GetEnumValue("ProjectableTypes", "OBJECT") and obj.base == Engine.GetEnumValue("ProjectableBases", "SYSTEMBODY")
			arrayTable.draw("tradeships_stationinfo2", Core.params.port_params, arrayTable.addKeys(pairs, {
				port =    function(k,_) return k end,
				label =   function(k,_) return k:GetLabel() end,
				parent =  function(k,_) return k:GetSystemBody().parent.name end,
				dist =    function(k,_) return k:DistanceTo(k:GetSystemBody().nearestJumpable.body) end,
				docks =   function(k,_) totals.docks = totals.docks + k.numDocks return k.numDocks end,
				busy_s =  function(k,_) totals.busy_s = totals.busy_s + k.numShipsDocked return k.numShipsDocked end,
				inbound = function(k,_) return inbound[k] end,
				landed =  function(_,v) totals.landed = totals.landed + v.landed return v.landed end,
				flow =    function(_,v) totals.flow = totals.flow + v.flow return v.flow end
			}),{
				{ name = "Port",       key = "label",  string = true               },
				{ name = "Parent",     key = "parent", string = true               },
				{ name = "Distance",   key = "dist",   fnc = distanceInAU          },
				{ name = "Docks",      key = "docks"                               },
				{ name = "Busy",       key = "busy",   fnc = format("%.2f")        },
				{ name = "Landed",     key = "busy_s"                              },
				{ name = "Calculated", key = "landed", fnc = format("%.2f")        },
				{ name = "Dock time",  key = "time",   fnc = format("%.2fh")       },
				{ name = "Inbound",    key = "inbound"                             },
				{ name = "Ship flow",  key = "flow",   fnc = format("%.2f ship/h") },
			},{
				totals = { totals },
				callbacks = {
					onClick = function(row)
						Game.systemView:SetSelectedObject(Engine.GetEnumValue("ProjectableTypes", "OBJECT"),
							Engine.GetEnumValue("ProjectableBases", "SYSTEMBODY"), row.port:GetSystemBody())
					end,
					isSelected = function(row)
						return sb_selected and Game.systemView:GetSelectedObject().ref == row.port:GetSystemBody()
					end
				}})
		end

		if ui.collapsingHeader("Local routes") then
			for shipname, params in pairs(Core.params.local_routes) do
				ui.text("  ")
				ui.sameLine()
				if ui.treeNode(shipname .. " (" .. #params .. ")") then
					arrayTable.draw("tradeships_" .. shipname .. "_info", params, ipairs, {
						{ name = "From",     key = "from",     fnc = method("GetLabel"), string = true },
						{ name = "To",       key = "to",       fnc = method("GetLabel"), string = true },
						{ name = "Duration", key = "duration", fnc = ui.Format.Duration                },
						{ name = "Distance", key = "distance", fnc = distanceInAU                      }
					})
					ui.treePop()
				end
			end
		end

		if ui.collapsingHeader("Hyperspace routes") then
			local function sysName(path)
				return path:GetStarSystem().name
			end
			local selected_in_sectorview = Game.sectorView:GetSelectedSystemPath()
			for shipname, params in pairs(Core.params.hyper_routes) do
				if ui.treeNode(shipname .. " (" .. #params .. ")") then
					arrayTable.draw("tradeships_" .. shipname .. "_hyperinfo", params, ipairs, {
						{ name = "From",           key = "from",           fnc = sysName,             string = true },
						{ name = "Distance",       key = "distance",       fnc = format("%.2f l.y.")                },
						{ name = "Fuel",           key = "fuel",           fnc = format("%.2f t")                   },
						{ name = "Duration",       key = "duration",       fnc = ui.Format.Duration                 },
						{ name = "Cloud Duration", key = "cloud_duration", fnc = ui.Format.Duration                 }
					},{
						callbacks = {
							onClick = function(row)
								Game.sectorView:SwitchToPath(row.from)
							end,
							isSelected = function(row)
								return row.from:IsSameSystem(selected_in_sectorview)
							end
						}})
					ui.treePop()
				end
			end
		end
		end

		if ui.collapsingHeader("All ships") then
			local ships = {}
			for ship, trader in pairs(Core.ships) do
				if ship:exists() then
					table.insert(ships, {
						ship = ship,
						label = ship:GetLabel(),
						status = trader.status,
						ts_error = trader.ts_error,
						cargo = ship.usedCargo,
						ai = ship:GetCurrentAICommand(),
						model = ship.shipId,
						port = trader.starport and trader.starport:GetLabel()
					})
				end
			end
			local obj = Game.systemView:GetSelectedObject()
			local ship_selected = obj.type == Engine.GetEnumValue("ProjectableTypes", "OBJECT") and obj.base == Engine.GetEnumValue("ProjectableBases", "SHIP")
			arrayTable.draw("tradeships_all", ships, ipairs, {
				{ name = "#",      key = "#"                       },
				{ name = "Label",  key = "label",    string = true },
				{ name = "Model",  key = "model",    string = true },
				{ name = "Status", key = "status",   string = true },
				{ name = "Error",  key = "ts_error", string = true },
				{ name = "Cargo",  key = "cargo"                   },
				{ name = "AI",     key = "ai",       string = true },
				{ name = "Port",   key = "port",     string = true }
			},
			{ callbacks = {
				onClick = function(row)
					if row.status ~= "hyperspace" and row.status ~= "hyperspace_out" then
						Game.systemView:SetSelectedObject(Engine.GetEnumValue("ProjectableTypes", "OBJECT"),
							Engine.GetEnumValue("ProjectableBases", "SHIP"), row.ship)
					end
				end,
				isSelected = function(row)
					return ship_selected and Game.systemView:GetSelectedObject().ref == row.ship
				end
			}
			})
		end
		if ui.collapsingHeader("Log") then
			local obj = Game.systemView:GetSelectedObject()
			local ship_selected = obj.type == Engine.GetEnumValue("ProjectableTypes", "OBJECT") and obj.base == Engine.GetEnumValue("ProjectableBases", "SHIP")
			search_text, _ = ui.inputText("Search log", search_text, {})
			arrayTable.draw("tradeships_log", Core.log, Core.log.iter(search_text ~= "" and function (row)
				return
					row.label and string.match(row.label, search_text) or
					row.msg and string.match(row.msg, search_text) or
					row.model and string.match(row.model, search_text)
			end),{
				{ name = "Time",    key = "time" , fnc = ui.Format.Datetime },
				{ name = "Label",   key = "label", string = true            },
				{ name = "Model",   key = "model", string = true            },
				{ name = "Message", key = "msg",   string = true            }
			},{
				callbacks = {
					onClick = function(row)
						local status = Core.ships[row.ship] and Core.ships[row.ship].status
						if status and status ~= "hyperspace" and status ~= "hyperspace_out" then
							Game.systemView:SetSelectedObject(Engine.GetEnumValue("ProjectableTypes", "OBJECT"),
								Engine.GetEnumValue("ProjectableBases", "SHIP"), row.ship)
						end
					end,
					isSelected = function(row)
						return ship_selected and obj.ref == row.ship
					end
				}})
		end
	end)


	infosize = ui.getCursorScreenPos().y
	local obj = Game.systemView:GetSelectedObject()
	if obj.type ~= Engine.GetEnumValue("ProjectableTypes", "NONE") and Core.ships[obj.ref] then
		local ship = obj.ref
		local trader = Core.ships[ship]

		if ui.collapsingHeader("Info:", {"DefaultOpen"}) then
			property("Trader: ", ship:GetShipType() .. " " .. ship:GetLabel())

			local status = trader.status
			if status == "docked" then
				status = status .. " (" .. trader.starport:GetLabel() .. ")"
			elseif status == "inbound" then
				local d = ui.Format.Distance(ship:DistanceTo(trader.starport))
				status = status .. " (" .. trader.starport:GetLabel() .. " - " .. d .. ")"
			end
			property("Status: ", status)

			if trader.fnc then
				property("Task: ", trader.fnc .. " in " .. ui.Format.Duration(trader.delay - Game.time))
			end
			property("Fuel: ", string.format("%.4f", ship.fuelMassLeft) .. "/" .. ShipDef[ship.shipId].fuelTankMass .. " t")
		end

		if ui.collapsingHeader("Internals:", {"DefaultOpen"}) then
			local equipItems = {}
			local total_mass = 0

			local equips = { "misc", "hyperspace", "laser" }
			for _,t in ipairs(equips) do
				for _,et in pairs(e[t]) do
					local count = ship:CountEquip(et)
					if count > 0 then
						local all_mass = count * et.capabilities.mass
						table.insert(equipItems, {
							name = et:GetName(),
							eq_type = t,
							count = count,
							mass = et.capabilities.mass,
							all_mass = all_mass
						})
						total_mass = total_mass + all_mass
					end
				end
			end

			---@type CargoManager
			local cargoMgr = ship:GetComponent('CargoManager')

			for name, info in pairs(cargoMgr.commodities) do
				local ct = CommodityType.GetCommodity(name)
				total_mass = total_mass + ct.mass * info.count
				table.insert(equipItems, {
					name = ct:GetName(),
					eq_type = "cargo",
					count = info.count,
					mass = ct.mass,
					all_mass = ct.mass * info.count
				})
			end

			local capacity = ShipDef[ship.shipId].capacity

			arrayTable.draw("tradeships_traderequipment", equipItems, ipairs, {
				{ name = "Name",        key = "name",     string = true       },
				{ name = "Type",        key = "eq_type",  string = true       },
				{ name = "Units",       key = "count"                         },
				{ name = "Unit's mass", key = "mass",     fnc = format("%dt") },
				{ name = "Total",       key = "all_mass", fnc = format("%dt") }
			},
			{ totals = {
				{ name = "Total:",    all_mass = total_mass            },
				{ name = "Capacity:", all_mass = capacity              },
				{ name = "Free:",     all_mass = capacity - total_mass },
			}})
		end
	end
	infosize = ui.getCursorScreenPos().y - infosize
	ui.endTabItem()
end)
