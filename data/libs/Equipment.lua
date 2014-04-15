local EquipType = {class="EquipType"}
local equipType_meta = { __index = EquipType }

function EquipType.New (specs)
	local obj = {}
	for i,v in pairs(specs) do
		obj[i] = v
	end
	setmetatable(obj, equipType_meta)
	if type(obj.slots) ~= "table" then
		obj.slots = {obj.slots}
	end
	return obj
end

function EquipType:GetDefaultSlot(ship)
	return self.slots[1]
end

function EquipType:IsValidSlot(ship, slot)
	for _, s in ipairs(self.slots) do
		if s == slot then
			return true
		end
	end
	return false
end

function __ApplyCapabilities(ship, capabilities, num, factor)
	if num <= 0 then return 0 end
	local factor = factor or 1
	for k,v in pairs(capabilities) do
		local full_name = k.."_cap"
		local prev = (ship:hasprop(full_name) and ship[full_name]) or 0
		ship:setprop(full_name, (factor*v)+prev)
	end
	return num
end

function EquipType:Install(ship, num, slot)
	return __ApplyCapabilities(ship, self.capabilities, num, 1)
end

function EquipType:Uninstall(ship, num, slot)
	return __ApplyCapabilities(ship, self.capabilities, num, -1)
end

cargo = {
	hydrogen = EquipType.New({
		name="Hydrogen", description="Hydrogen",
		slots="cargo", price=100, capabilities={mass=1},
		economy_type="mining"
	}),
	liquid_oxygen = EquipType.New({
		name="Liquid Oxygen", description="Liquid Oxygen",
		slots="cargo", price=150, capabilities={mass=1},
		economy_type="mining"
	}),
	water = EquipType.New({
		name="Water", description="",
		slots="cargo", price=120, capabilities={mass=1},
		economy_type="mining"
	}),
	carbon_ore = EquipType.New({
		name="Carbon Ore", description="",
		slots="cargo", price=500, capabilities={mass=1},
		economy_type="mining"
	}),
	metal_ore = EquipType.New({
		name="Metal Ore", description="",
		slots="cargo", price=300, capabilities={mass=1},
		economy_type="mining"
	}),
	metal_alloys = EquipType.New({
		name="Metal Alloys", description="",
		slots="cargo", price=800, capabilities={mass=1},
		economy_type="industry"
	}),
	precious_metals = EquipType.New({
		name="Precious Metals", description="",
		slots="cargo", price=18000, capabilities={mass=1},
		economy_type="industry"
	}),
	plastics = EquipType.New({
		name="Plastics", description="",
		slots="cargo", price=1200, capabilities={mass=1},
		economy_type="industry"
	}),
	fruit_and_veg = EquipType.New({
		name="Fruits and Vegetables", description="",
		slots="cargo", price=1200, capabilities={mass=1},
		economy_type="agriculture"
	}),
	animal_meat = EquipType.New({
		name="Animal Meat", description="",
		slots="cargo", price=1800, capabilities={mass=1},
		economy_type="agriculture"
	}),
	live_animals = EquipType.New({
		name="Live Animals", description="",
		slots="cargo", price=3200, capabilities={mass=1},
		economy_type="agriculture"
	}),
	liquor = EquipType.New({
		name="Liquor", description="",
		slots="cargo", price=800, capabilities={mass=1},
		economy_type="agriculture"
	}),
	grain = EquipType.New({
		name="Grain", description="",
		slots="cargo", price=1000, capabilities={mass=1},
		economy_type="agriculture"
	}),
	slaves = EquipType.New({
		name="Slaves", description="",
		slots="cargo", price=23200, capabilities={mass=1},
		economy_type="agriculture"
	}),
	textiles = EquipType.New({
		name="Textiles", description="",
		slots="cargo", price=850, capabilities={mass=1},
		economy_type="industry"
	}),
	fertilizer = EquipType.New({
		name="Fertilizer", description="",
		slots="cargo", price=400, capabilities={mass=1},
		economy_type="industry"
	}),
	medicines = EquipType.New({
		name="Medicines", description="",
		slots="cargo", price=2200, capabilities={mass=1},
		economy_type="industry"
	}),
	consumer_goods = EquipType.New({
		name="Consumer Goods", description="",
		slots="cargo", price=14000, capabilities={mass=1},
		economy_type="industry"
	}),
	computers = EquipType.New({
		name="Computers", description="",
		slots="cargo", price=8000, capabilities={mass=1},
		economy_type="industry"
	}),
	rubbish = EquipType.New({
		name="Rubbish", description='',
		slots="cargo", price=-10, capabilities={mass=1},
		economy_type="industry"
	}),
	radioactives = EquipType.New({
		name="Radioactive waste", description='',
		slots="cargo", price=-35, capabilities={mass=1},
		economy_type="industry"
	}),
	narcotics = EquipType.New({
		name="Narcotics", description='',
		slots="cargo", price=15700, capabilities={mass=1},
		economy_type="industry"
	}),
	nerve_gas = EquipType.New({
		name="Nerve gas", description='',
		slots="cargo", price=26500, capabilities={mass=1},
		economy_type="industry"
	}),
	military_fuel = EquipType.New({
		name="Military Fuel", description='',
		slots="cargo", price=6000, capabilities={mass=1},
		economy_type="industry"
	}),
	robots = EquipType.New({
		name="Robots", description='',
		slots="cargo", price=6300, capabilities={mass=1},
		economy_type="industry"
	}),
	hand_weapons = EquipType.New({
		name="Hand weapons", description='',
		slots="cargo", price=12400, capabilities={mass=1},
		economy_type="industry"
	}),
	air_processors = EquipType.New({
		name="Air Processors", description='',
		slots="cargo", price=2000, capabilities={mass=1},
		economy_type="industry"
	}),
	farm_machinery = EquipType.New({
		name="Farm machinery", description="",
		slots="cargo", price=1100, capabilities={mass=1},
		economy_type="industry"
	}),
	mining_machinery = EquipType.New({
		name="Mining machinery", description="",
		slots="cargo", price=1200, capabilities={mass=1},
		economy_type="industry"
	}),
	battle_weapons = EquipType.New({
		name="Battle weapons", description="",
		slots="cargo", price=22000, capabilities={mass=1},
		economy_type="industry"
	}),
	industrial_machinery = EquipType.New({
		name="Industrial machinery", description="Industrial machinery",
		slots="cargo", price=1300, capabilities={mass=1},
		economy_type="industry"
	}),
}
cargo.liquid_oxygen.requirements = { cargo.water, cargo.industrial_machinery }
cargo.battle_weapons.requirements = { cargo.metal_alloys, cargo.industrial_machinery }
cargo.farm_machinery.requirements = { cargo.metal_alloys, cargo.robots }
cargo.mining_machinery.requirements = { cargo.metal_alloys, cargo.robots }
cargo.industrial_machinery.requirements = { cargo.metal_alloys, cargo.robots }
cargo.air_processors.requirements = { cargo.plastics, cargo.industrial_machinery }
cargo.robots.requirements = { cargo.plastics, cargo.computers }
cargo.hand_weapons.requirements = { cargo.computers }
cargo.computers.requirements = { cargo.precious_metals, cargo.industrial_machinery }
cargo.metal_ore.requirements = { cargo.mining_machinery }
cargo.carbon_ore.requirements = { cargo.mining_machinery }
cargo.metal_alloys.requirements = { cargo.mining_machinery }
cargo.precious_metals.requirements = { cargo.mining_machinery }
cargo.water.requirements = { cargo.mining_machinery }
cargo.plastics.requirements = { cargo.carbon_ore, cargo.industrial_machinery }
cargo.fruit_and_veg.requirements = { cargo.farm_machinery, cargo.fertilizer }
cargo.animal_meat.requirements = { cargo.farm_machinery, cargo.fertilizer }
cargo.live_animals.requirements = { cargo.farm_machinery, cargo.fertilizer }
cargo.liquor.requirements = { cargo.farm_machinery, cargo.fertilizer }
cargo.grain.requirements = { cargo.farm_machinery, cargo.fertilizer }
cargo.textiles.requirements = { cargo.plastics }
cargo.military_fuel.requirements = { cargo.hydrogen }
cargo.fertilizer.requirements = { cargo.carbon_ore }
cargo.medicines.requirements = { cargo.computers, cargo.carbon_ore }
cargo.consumer_goods.requirements = { cargo.plastics, cargo.textiles }

local misc = {}
misc.missile_unguided = EquipType.New({
	name="Unguided missile", description="",
	slots="missile", price=3000, capabilities={mass=1, missile=1}
})
misc.missile_guided = EquipType.New({
	name="Guided missile", description="",
	slots="missile", price=5000, capabilities={mass=1}
})
misc.missile_smart = EquipType.New({
	name="Smart missile", description="",
	slots="missile", price=9500, capabilities={mass=1}
})
misc.missile_naval = EquipType.New({
	name="Naval missile", description="",
	slots="missile", price=16000, capabilities={mass=1}
})
misc.atmospheric_shielding = EquipType.New({
	name="Atmospheric shielding", description="",
	slots="atmo_shield", price=20000, capabilities={mass=1, atmo_shield=1}
})
misc.ecm_basic = EquipType.New({
	name="Basic ECM", description="",
	slots="ecm", price=600000, capabilities={mass=2, ecm_power=2, ecm_recharge=5}
})
misc.ecm_advanced = EquipType.New({
	name="Advanced ECM", description="",
	slots="ecm", price=1520000, capabilities={mass=2, ecm_power=3, ecm_recharge=5}
})
misc.scanner = EquipType.New({
	name="Scanner", description="",
	slots="scanner", price=68000, capabilities={mass=1, scanner=1}
})
misc.cabin = EquipType.New({
	name="Cabin", description="",
	slots="cabin", price=135000, capabilities={mass=1, cabin=1}
})
misc.shield_generator = EquipType.New({
	name="Shield generator", description="",
	slots="shield", price=250000, capabilities={mass=4, shield=1}
})
misc.laser_cooling_booster = EquipType.New({
	name="Laser cooling booster", description="",
	slots="laser_cooler", price=38000, capabilities={mass=1, laser_cooler=2}
})
misc.cargo_life_support = EquipType.New({
	name="Cargo life support", description="",
	slots="cargo_life_support", price=70000, capabilities={mass=1, cargo_life_support=1}
})
misc.autopilot = EquipType.New({
	name="Autopilot", description="",
	slots="autopilot", price=140000, capabilities={mass=1, set_speed=1, autopilot=1}
})
misc.radar_mapper = EquipType.New({
	name="Radar mapper", description="",
	slots="radar", price=90000, capabilities={mass=1, radar_mapper=1}
})
misc.fuel_scoop = EquipType.New({
	name="Fuel scoop", description="",
	slots="fuel_scoop", price=350000, capabilities={mass=6, fuel_scoop=1}
})
misc.cargo_scoop = EquipType.New({
	name="Cargo scoop", description="",
	slots="cargo_scoop", price=390000, capabilities={mass=7, cargo_scoop=1}
})
misc.hypercloud_analyzer = EquipType.New({
	name="Hypercloud analyzer", description="",
	slots="hypercloud", price=150000, capabilities={mass=1, hypercloud_analyzer=1}
})
misc.shield_energy_booster = EquipType.New({
	name="Shield energy booster", description="",
	slots="energy_booster", price=1000000, capabilities={mass=8, shield_energy_booster=1}
})
misc.hull_autorepair = EquipType.New({
	name="Hull autorepair", description="",
	slots="hull_autorepair", price=1600000, capabilities={mass=40, hull_autorepair=1}
})

local hyperspace = {}
hyperspace.hyperdrive_1 = EquipType.new({
	name="Hyperdrive class 1", description="", fuel=cargo.hydrogen,
	slots="engine", price=70000, capacities={mass=4, hyperclass=1},
})
hyperspace.hyperdrive_2 = EquipType.new({
	name="Hyperdrive class 2", description="", fuel=cargo.hydrogen,
	slots="engine", price=130000, capacities={mass=10, hyperclass=2}
})
hyperspace.hyperdrive_3 = EquipType.new({
	name="Hyperdrive class 3", description="", fuel=cargo.hydrogen,
	slots="engine", price=250000, capacities={mass=20, hyperclass=3}
})
hyperspace.hyperdrive_4 = EquipType.new({
	name="Hyperdrive class 4", description="", fuel=cargo.hydrogen,
	slots="engine", price=500000, capacities={mass=40, hyperclass=4}
})
hyperspace.hyperdrive_5 = EquipType.new({
	name="Hyperdrive class 5", description="", fuel=cargo.hydrogen,
	slots="engine", price=1000000, capacities={mass=120, hyperclass=5}
})
hyperspace.hyperdrive_6 = EquipType.new({
	name="Hyperdrive class 6", description="", fuel=cargo.hydrogen,
	slots="engine", price=2000000, capacities={mass=225, hyperclass=6}
})
hyperspace.hyperdrive_7 = EquipType.new({
	name="Hyperdrive class 7", description="", fuel=cargo.hydrogen,
	slots="engine", price=3000000, capacities={mass=400, hyperclass=7}
})
hyperspace.hyperdrive_8 = EquipType.new({
	name="Hyperdrive class 8", description="", fuel=cargo.hydrogen,
	slots="engine", price=6000000, capacities={mass=580, hyperclass=8}
})
hyperspace.hyperdrive_9 = EquipType.new({
	name="Hyperdrive class 9", description="", fuel=cargo.hydrogen,
	slots="engine", price=12000000, capacities={mass=740, hyperclass=9}
})
hyperspace.hyperdrive_mil1 = EquipType.new({
	name="Hyperdrive military class 1", description="", fuel=cargo.military_fuel,
	slots="engine", price=2300000, capacities={mass=3, hyperclass=1}
})
hyperspace.hyperdrive_mil2 = EquipType.new({
	name="Hyperdrive military class 2", description="", fuel=cargo.military_fuel,
	slots="engine", price=4700000, capacities={mass=8, hyperclass=2}
})
hyperspace.hyperdrive_mil3 = EquipType.new({
	name="Hyperdrive military class 3", description="", fuel=cargo.military_fuel,
	slots="engine", price=8500000, capacities={mass=16, hyperclass=3}
})
hyperspace.hyperdrive_mil4 = EquipType.new({
	name="Hyperdrive military class 4", description="", fuel=cargo.military_fuel,
	slots="engine", price=21400000, capacities={mass=30, hyperclass=4}
})

local laser = {}
laser.pulsecannon_1mw = EquipType.New({
	name="Pulse cannon (1MW)", description="",
	price=60000, capabilities={mass=1},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, dual=false, mining=false, color={1, 0.2, 0.2, 1}
	}
})
laser.pulsecannon_dual_1mw = EquipType.New({
	name="Dual pulse cannon (1MW)", description="",
	price=110000, capabilities={mass=4},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, dual=true, mining=false, color={1, 0.2, 0.2, 1}
	}
})
laser.pulsecannon_2mw = EquipType.New({
	name="Pulse cannon (2MW)", description="",
	price=100000, capabilities={mass=3},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.25, length=30,
		width=5, dual=false, mining=false, color={1, 0.5, 0.2, 1}
	}
})
laser.pulsecannon_rapid_2mw = EquipType.New({
	name="Rapid pulse cannon (2MW)", description="",
	price=180000, capabilities={mass=7},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.13, length=30,
		width=5, dual=false, mining=false, color={1, 0.5, 0.2, 1}
	}
})
laser.pulsecannon_4mw = EquipType.New({
	name="Pulse cannon (4MW)", description="",
	price=220000, capabilities={mass=10},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=4000, rechargeTime=0.25, length=30,
		width=5, dual=false, mining=false, color={1, 1, 0.2, 1}
	}
})
laser.pulsecannon_10mw = EquipType.New({
	name="Pulse cannon (10MW)", description="",
	price=490000, capabilities={mass=30},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=10000, rechargeTime=0.25, length=30,
		width=5, dual=false, mining=false, color={0.2, 1, 0.2, 1}
	}
})
laser.pulsecannon_20mw = EquipType.New({
	name="Pulse cannon (20MW)", description="",
	price=1200000, capabilities={mass=65},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=20000, rechargeTime=0.25, length=30,
		width=5, dual=false, mining=false, color={0.1, 0.2, 1, 1}
	}
})
laser.miningcannon_17mw = EquipType.New({
	name="Pulse cannon (20MW)", description="",
	price=1060000, capabilities={mass=10},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=17000, rechargeTime=2, length=30,
		width=5, dual=false, mining=true, color={0.2, 0.5, 0, 1}
	}
})
laser.small_plasma_accelerator = EquipType.New({
	name="Small plasma accelerator", description="",
	price=12000000, capabilities={mass=22},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=50000, rechargeTime=0.3, length=42,
		width=7, dual=false, mining=false, color={0.2, 1, 1, 1}
	}
})
laser.large_plasma_accelerator = EquipType.New({
	name="Large plasma accelerator", description="",
	price=39000000, capabilities={mass=50},
	slots = {"laser_front", "laser_rear"},
	laser_stats = {
		lifespan=8, speed=1000, damage=100000, rechargeTime=0.3, length=42,
		width=7, dual=false, mining=false, color={0.5, 1, 1, 1}
	}
})
local equipment = {
    cargo=cargo,
    laser=laser,
    hyperspace=hyperspace,
    misc=misc,
    EquipType=EquipType
}

return equipment
