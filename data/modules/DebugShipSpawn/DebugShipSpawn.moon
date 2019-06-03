
Game = require 'Game'
Space = require 'Space'
Ship = require 'Ship'
ShipDef = require 'ShipDef'
Equipment = require 'Equipment'

import Vector2, Color from _G
ui = require 'pigui.pigui'

ship_defs = {}

update_ship_def_table = ->
    for name in pairs ShipDef
        table.insert ship_defs, name
        print "Ship Def found: #{name}"
    table.sort ship_defs

update_ship_def_table!
print #ship_defs

selected_ship_type = 0
draw_ship_types = ->
    for i, ship in ipairs ship_defs
        if ui.selectable ship, selected_ship_type == i
            selected_ship_type = i

draw_ship_info = =>
    ui.text "Ship Info"
    ui.separator!
    ui.columns 2, 'ship_info', true
    ui.text "Name:"
    ui.nextColumn!
    ui.text @name
    ui.nextColumn!

    ui.text "Manufacturer:"
    ui.nextColumn!
    ui.text @manufacturer
    ui.nextColumn!

    ui.text "Ship Class:"
    ui.nextColumn!
    ui.text @shipClass
    ui.nextColumn!

ai_opt_selected = 1
ai_options = {
    "FlyTo", "Kamikaze", "Kill"
}

draw_ai_info = ->
    for i, opt in ipairs ai_options
        if ui.selectable opt, ai_opt_selected == i
            ai_opt_selected = i

spawn_distance = 10
ship_spawn_debug_window = ->
    ui.child 'ship_list', Vector2(150, 0), draw_ship_types

    ship_name = ship_defs[selected_ship_type]
    ship = ShipDef[ship_name] if ship_name

    ui.sameLine!
    if ship then ui.group ->
        ui.child 'ship_info', Vector2(-150, -ui.getFrameHeightWithSpacing!), ->
            draw_ship_info ship

        ui.sameLine!
        ui.child 'ai_info', Vector2(150, -ui.getFrameHeightWithSpacing!), draw_ai_info

        if ui.button "Spawn", Vector2(0, 0)
            new_ship = Space.SpawnShipNear(ship_name, Game.player, spawn_distance, spawn_distance)
            new_ship\AddEquip Equipment.laser.pulsecannon_dual_1mw
            new_ship\AddEquip Equipment.misc.laser_cooling_booster
            new_ship\AddEquip Equipment.misc.atmospheric_shielding
            new_ship\SetLabel Ship.MakeRandomLabel !
            ai_method_name = "AI#{ai_options[ai_opt_selected]}"
            new_ship[ai_method_name] new_ship, Game.player

        ui.sameLine!

        ui.text "Spawn Distance:"
        ui.sameLine!
        spawn_distance = ui.sliderFloat("#spawn_distance", spawn_distance, 0.5, 50, "%.1fkm")

displayDebugWindow = false
ui.registerModule 'game', ->
    if ui.isKeyReleased(ui.keys.f11) and ui.ctrlHeld!
        displayDebugWindow = not displayDebugWindow

    if displayDebugWindow and Game.CurrentView() == "world"
        ui.withStyleColors { "WindowBg": Color(15, 15, 16, 240) }, ->
            ui.window "Ship Spawn Debug", {}, ship_spawn_debug_window
