
Game = require 'Game'
Space = require 'Space'
Ship = require 'Ship'
ShipDef = require 'ShipDef'
Timer = require 'Timer'
Equipment = require 'Equipment'

import Vector2 from _G

ui = require 'pigui'
debug_ui = require 'pigui.views.debug'

ship_defs = {}

update_ship_def_table = ->
    for name in pairs ShipDef
        table.insert ship_defs, name
        print "Ship Def found: #{name}"
    table.sort ship_defs

update_ship_def_table!
print #ship_defs

selected_ship_type = 1
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

	ui.columns 1, ''

ai_opt_selected = 1
ai_options = {
    "FlyTo", "Kamikaze", "Kill"
}

-- ui.combo is zero-based
missile_selected = 0
missile_names = {
	"Guided Missile", "Unguided Missile", "Smart Missile"
}
missile_types = {
	"missile_guided",
	"missile_unguided",
	"missile_smart",
}

draw_ai_info = ->
    for i, opt in ipairs ai_options
        if ui.selectable opt, ai_opt_selected == i
            ai_opt_selected = i

spawn_distance = 5.0 -- km

spawn_ship_free = (ship_name, ai_option, equipment) ->
	new_ship = with Space.SpawnShipNear ship_name, Game.player, spawn_distance, spawn_distance
		\SetLabel Ship.MakeRandomLabel!
		\AddEquip equip for equip in *equipment
		\UpdateEquipStats!

	-- Invoke the specified AI method on the new ship.
	new_ship["AI#{ai_option}"] new_ship, Game.player

spawn_ship_docked = (ship_name, ai_option, equipment) ->
	new_ship = with Space.SpawnShipDocked ship_name, Game.player\GetNavTarget!
		\SetLabel Ship.MakeRandomLabel!
		\AddEquip equip for equip in *equipment
		\UpdateEquipStats!

	-- Invoke the specified AI method on the new ship.
	new_ship["AI#{ai_option}"] new_ship, Game.player

-- Spawn a missile attacking the player's current combat target
do_spawn_missile = (type) ->
	if Game.player\IsDocked!
		return nil

	new_missile = with Game.player\SpawnMissile type
		\AIKamikaze Game.player\GetCombatTarget!

	Timer\CallEvery 2, ->
		if new_missile\exists!
			new_missile\Arm!

		return true

ship_equip = {
	Equipment.laser.pulsecannon_dual_1mw
	Equipment.misc.laser_cooling_booster
	Equipment.misc.atmospheric_shielding
}

set_player_ship_type = (shipType) ->
	item_types = { Equipment.misc, Equipment.laser, Equipment.hyperspace }
	equip = [item for type in *item_types for _, item in pairs(type) for i=1, Game.player\CountEquip item]

	with Game.player
		\SetShipType shipType
		\AddEquip item for item in *equip
		\UpdateEquipStats!

ship_spawn_debug_window = ->
    ui.child 'ship_list', Vector2(150, 0), draw_ship_types

    ship_name = ship_defs[selected_ship_type]
	ship = ShipDef[ship_name] if ship_name
	return unless ship

    ui.sameLine!
	ui.group ->
		spawner_group_height = ui.getFrameHeightWithSpacing! * 3 + ui.getTextLineHeightWithSpacing!
        ui.child 'ship_info', Vector2(0, -spawner_group_height), ->
			draw_ship_info ship
			if ui.button "Set Player Ship Type", Vector2(0, 0)
				set_player_ship_type ship_name

			ui.spacing!
			ui.separator!
			ui.spacing!

			draw_ai_info!

		if ui.button "Spawn Ship", Vector2(0, 0)
			spawn_ship_free ship_name, ai_options[ai_opt_selected], ship_equip

		nav_target = Game.player\GetNavTarget!
		if nav_target and nav_target\isa "SpaceStation"
			ui.sameLine!
			if ui.button "Spawn Docked", Vector2(0, 0)
				spawn_ship_docked ship_name, ai_options[ai_opt_selected], ship_equip

		SectorView = Game.sectorView
		if not Game.player\GetDockedWith!
			ui.sameLine!
			if nav_target and nav_target\isa "SpaceStation"
				if ui.button "Teleport To", Vector2(0, 0)
					Game.player\SetDockedWith nav_target

			if SectorView\GetSelectedSystemPath! and Game.system and not SectorView\GetSelectedSystemPath!\IsSameSystem(Game.system.path)
				if ui.button "Hyperjump To", Vector2(0, 0)
					Game.player\InitiateHyperjumpTo(SectorView\GetSelectedSystemPath(), 1.0, 0.0, {})

		if Game.player\GetCombatTarget!
			if ui.button "Spawn##Missile", Vector2(0, 0)
				do_spawn_missile missile_types[missile_selected + 1]
			ui.sameLine 0, 2

		ui.nextItemWidth -1.0
		_, missile_selected = ui.combo "##missile_type", missile_selected, missile_names

		ui.text "Spawn Distance:"
		ui.nextItemWidth -1.0
		spawn_distance = ui.sliderFloat("##spawn_distance", spawn_distance, 0.5, 20, "%.1fkm")

debug_ui.registerTab "Ship Spawner", ->
	unless Game.player and Game.CurrentView() == "world" -- when not in a game, Game.player will return nil
		return nil

	if ui.beginTabItem "Ship Spawner"
		ship_spawn_debug_window!

		ui.endTabItem!

		if ui.isKeyReleased(string.byte 'r') and ui.ctrlHeld!
			package.reimport '.DebugShipSpawn'

ui.registerModule "game", ->
	unless Game.CurrentView() == "world"
		return nil

	if ui.isKeyReleased(ui.keys.f12) and ui.ctrlHeld!
		spawn_ship_free "kanara", "Kill", ship_equip
