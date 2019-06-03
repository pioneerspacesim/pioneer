local Game = require('Game')
local Space = require('Space')
local Ship = require('Ship')
local ShipDef = require('ShipDef')
local Equipment = require('Equipment')
local Vector2, Color
do
  local _obj_0 = _G
  Vector2, Color = _obj_0.Vector2, _obj_0.Color
end
local ui = require('pigui.pigui')
local ship_defs = { }
local update_ship_def_table
update_ship_def_table = function()
  for name in pairs(ShipDef) do
    table.insert(ship_defs, name)
    print("Ship Def found: " .. tostring(name))
  end
  return table.sort(ship_defs)
end
update_ship_def_table()
print(#ship_defs)
local selected_ship_type = 0
local draw_ship_types
draw_ship_types = function()
  for i, ship in ipairs(ship_defs) do
    if ui.selectable(ship, selected_ship_type == i) then
      selected_ship_type = i
    end
  end
end
local draw_ship_info
draw_ship_info = function(self)
  ui.text("Ship Info")
  ui.separator()
  ui.columns(2, 'ship_info', true)
  ui.text("Name:")
  ui.nextColumn()
  ui.text(self.name)
  ui.nextColumn()
  ui.text("Manufacturer:")
  ui.nextColumn()
  ui.text(self.manufacturer)
  ui.nextColumn()
  ui.text("Ship Class:")
  ui.nextColumn()
  ui.text(self.shipClass)
  return ui.nextColumn()
end
local ai_opt_selected = 1
local ai_options = {
  "FlyTo",
  "Kamikaze",
  "Kill"
}
local draw_ai_info
draw_ai_info = function()
  for i, opt in ipairs(ai_options) do
    if ui.selectable(opt, ai_opt_selected == i) then
      ai_opt_selected = i
    end
  end
end
local spawn_distance = 10
local ship_spawn_debug_window
ship_spawn_debug_window = function()
  ui.child('ship_list', Vector2(150, 0), draw_ship_types)
  local ship_name = ship_defs[selected_ship_type]
  local ship
  if ship_name then
    ship = ShipDef[ship_name]
  end
  ui.sameLine()
  if ship then
    return ui.group(function()
      ui.child('ship_info', Vector2(-150, -ui.getFrameHeightWithSpacing()), function()
        return draw_ship_info(ship)
      end)
      ui.sameLine()
      ui.child('ai_info', Vector2(150, -ui.getFrameHeightWithSpacing()), draw_ai_info)
      if ui.button("Spawn", Vector2(0, 0)) then
        local new_ship = Space.SpawnShipNear(ship_name, Game.player, spawn_distance, spawn_distance)
        new_ship:AddEquip(Equipment.laser.pulsecannon_dual_1mw)
        new_ship:AddEquip(Equipment.misc.laser_cooling_booster)
        new_ship:AddEquip(Equipment.misc.atmospheric_shielding)
        new_ship:SetLabel(Ship.MakeRandomLabel())
        local ai_method_name = "AI" .. tostring(ai_options[ai_opt_selected])
        new_ship[ai_method_name](new_ship, Game.player)
      end
      ui.sameLine()
      ui.text("Spawn Distance:")
      ui.sameLine()
      spawn_distance = ui.sliderFloat("#spawn_distance", spawn_distance, 0.5, 50, "%.1fkm")
    end)
  end
end
local displayDebugWindow = false
return ui.registerModule('game', function()
  if ui.isKeyReleased(ui.keys.f11) and ui.ctrlHeld() then
    displayDebugWindow = not displayDebugWindow
  end
  if displayDebugWindow and Game.CurrentView() == "world" then
    return ui.withStyleColors({
      ["WindowBg"] = Color(15, 15, 16, 240)
    }, function()
      return ui.window("Ship Spawn Debug", { }, ship_spawn_debug_window)
    end)
  end
end)
