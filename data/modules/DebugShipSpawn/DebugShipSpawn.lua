local Game = require('Game')
local Space = require('Space')
local Ship = require('Ship')
local ShipDef = require('ShipDef')
local Timer = require('Timer')
local Equipment = require('Equipment')
local Vector2
Vector2 = _G.Vector2
local ui = require('pigui')
local debug_ui = require('pigui.views.debug')
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
  ui.nextColumn()
  return ui.columns(1, '')
end
local ai_opt_selected = 1
local ai_options = {
  "FlyTo",
  "Kamikaze",
  "Kill"
}
local missile_selected = 0
local missile_names = {
  "Guided Missile",
  "Unguided Missile",
  "Smart Missile"
}
local missile_types = {
  "missile_guided",
  "missile_unguided",
  "missile_smart"
}
local draw_ai_info
draw_ai_info = function()
  for i, opt in ipairs(ai_options) do
    if ui.selectable(opt, ai_opt_selected == i) then
      ai_opt_selected = i
    end
  end
end
local spawn_distance = 5.0
local spawn_ship_free
spawn_ship_free = function(ship_name, ai_option, equipment)
  local new_ship
  do
    local _with_0 = Space.SpawnShipNear(ship_name, Game.player, spawn_distance, spawn_distance)
    _with_0:SetLabel(Ship.MakeRandomLabel())
    for _index_0 = 1, #equipment do
      local equip = equipment[_index_0]
      _with_0:AddEquip(equip)
    end
    _with_0:UpdateEquipStats()
    new_ship = _with_0
  end
  return new_ship["AI" .. tostring(ai_option)](new_ship, Game.player)
end
local spawn_ship_docked
spawn_ship_docked = function(ship_name, ai_option, equipment)
  local new_ship
  do
    local _with_0 = Space.SpawnShipDocked(ship_name, Game.player:GetNavTarget())
    _with_0:SetLabel(Ship.MakeRandomLabel())
    for _index_0 = 1, #equipment do
      local equip = equipment[_index_0]
      _with_0:AddEquip(equip)
    end
    _with_0:UpdateEquipStats()
    new_ship = _with_0
  end
  return new_ship["AI" .. tostring(ai_option)](new_ship, Game.player)
end
local do_spawn_missile
do_spawn_missile = function(type)
  if Game.player:IsDocked() then
    return nil
  end
  local new_missile
  do
    local _with_0 = Game.player:SpawnMissile(type)
    _with_0:AIKamikaze(Game.player:GetCombatTarget())
    new_missile = _with_0
  end
  return Timer:CallEvery(2, function()
    if new_missile:exists() then
      new_missile:Arm()
    end
    return true
  end)
end
local ship_spawn_debug_window
ship_spawn_debug_window = function()
  ui.child('ship_list', Vector2(150, 0), draw_ship_types)
  local ship_name = ship_defs[selected_ship_type]
  local ship
  if ship_name then
    ship = ShipDef[ship_name]
  end
  local ship_equip = {
    Equipment.laser.pulsecannon_dual_1mw,
    Equipment.misc.laser_cooling_booster,
    Equipment.misc.atmospheric_shielding
  }
  ui.sameLine()
  if ship then
    return ui.group(function()
      local spawner_group_height = ui.getFrameHeight() * 2 + ui.getFrameHeightWithSpacing()
      ui.child('ship_info', Vector2(0, -spawner_group_height), function()
        draw_ship_info(ship)
        ui.spacing()
        ui.separator()
        ui.spacing()
        return draw_ai_info()
      end)
      if ui.button("Spawn Ship", Vector2(0, 0)) then
        spawn_ship_free(ship_name, ai_options[ai_opt_selected], ship_equip)
      end
      local nav_target = Game.player:GetNavTarget()
      if nav_target and nav_target:isa("SpaceStation") then
        ui.sameLine()
        if ui.button("Spawn Docked", Vector2(0, 0)) then
          spawn_ship_docked(ship_name, ai_options[ai_opt_selected], ship_equip)
        end
      end
      if Game.player:GetCombatTarget() then
        ui.sameLine()
        if ui.button("Spawn Missile", Vector2(0, 0)) then
          do_spawn_missile(missile_types[missile_selected + 1])
        end
      end
      ui.nextItemWidth(-1.0)
      local _
      _, missile_selected = ui.combo("##missile_type", missile_selected, missile_names)
      ui.text("Spawn Distance:")
      ui.nextItemWidth(-1.0)
      spawn_distance = ui.sliderFloat("##spawn_distance", spawn_distance, 0.5, 20, "%.1fkm")
    end)
  end
end
return debug_ui.registerTab("Ship Spawner", function()
  if not (Game.player and Game.CurrentView() == "world") then
    return nil
  end
  if ui.beginTabItem("Ship Spawner") then
    ship_spawn_debug_window()
    ui.endTabItem()
    if ui.isKeyReleased(string.byte('r')) and ui.ctrlHeld() then
      package.reimport('.DebugShipSpawn')
    end
  end
  if ui.isKeyReleased(ui.keys.f12) and ui.ctrlHeld() then
    return nil
  end
end)
