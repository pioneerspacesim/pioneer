-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local theme = {}

local rescaleUI = require 'pigui.libs.rescale-ui'
local Vector2 = _G.Vector2
local Color = _G.Color

-- theme.styleColors
-- This table provides a coherent theme palette that can be referenced by
-- component-specific semantic colors for consistent styling across the UI
--
-- The gray_*, primary_*, and accent_* colors define nine style breakpoints,
-- whereas the success_* (green), warning_* (orange) and danger_* (red)
-- colors define only five due to their more limited utility.
--
-- Ideally we'd like to drop a few of these breakpoints and consolidate the
-- main UI further, but the significant number of elements in need of color
-- makes that somewhat difficult.
--
-- Most colors in this table are defined in terms of a palette containing up to
-- nine luminance breakpoints. These breakpoints are intended to ensure that
-- UI styling is clear and accessible, regardless of colorblindness or other
-- mitigating factors.
local styleColors = {
	black			= Color "000000",
	transparent		= Color "00000000",
	white	 		= Color "FFFFFF",
	unknown			= Color "FF00FF",

	-- Grays are intended for foreground text and design elements over the game
	-- directly or over a panel background color of equal or darker luminance
	-- Generated with the Material Design 2 tool

	gray_100 		= Color "F4F4F5", --<-- Main color for the palette tool
	gray_200 		= Color "ECECED",
	gray_300 		= Color "DEDEDF",
	gray_400 		= Color "BBBBBB",
	gray_500 		= Color "9B9B9C",
	gray_600 		= Color "727273",
	gray_700 		= Color "5F5F5F",

	-- Panel colors are partially based on the Material Design system, but
	-- shifted about half a shade darker to be a good background for the
	-- primary color

	panel_500 		= Color "73757D",
	panel_600 		= Color "5C5F66",
	panel_700 		= Color "40434A",
	panel_800 		= Color "31333A",
	panel_900 		= Color "181A20",

	-- Primary color is used for all interactable elements and certain headers
	-- and other dividing elements.
	-- Generated with the Material Design 2 tool

	primary_100		= Color "C7CCDE",
	primary_200		= Color "A4ABC8",
	primary_300		= Color "818BB1",
	primary_400		= Color "6771A1",
	primary_500		= Color "4F5992", --<-- Main color for the palette tool
	primary_600		= Color "485189",
	primary_700		= Color "40477D",
	primary_800		= Color "383C71",
	primary_900		= Color "2C2D58",

	-- "Alternate" primary colors with significantly more saturation for use
	-- as small foreground elements like checkmarks and grabs.

	primary_300a	= Color "697ED3",
	primary_500a	= Color "4C5AA4",
	primary_700a	= Color "3A4688",

	accent_100		= Color "BBDDFF",
	accent_200		= Color "8FC8FF",
	accent_300		= Color "62B2FF",
	accent_400		= Color "40A1FF",
	accent_500		= Color "2491FF",
	accent_600		= Color "2882F1",
	accent_700		= Color "2870DD",
	accent_800		= Color "275FCB",
	accent_900		= Color "253FAB",

	success_100		= Color "CAF8A8",
	success_300		= Color "77EE21",
	success_500		= Color "5ACC0A",
	success_700		= Color "317005",
	success_900		= Color "102502",

	warning_100		= Color "FFD391",
	warning_300		= Color "FFA431",
	warning_500		= Color "FF8000",
	warning_700		= Color "894400",
	warning_900		= Color "401F00",

	danger_300		= Color "EB3737",
	danger_500		= Color "AD1F1F",
	danger_700		= Color "8F1416",
	danger_900		= Color "790606",

}

theme.styleColors = styleColors

theme.buttonColors = {
	default = {
		normal  = styleColors.primary_700,
		hovered = styleColors.primary_500,
		active  = styleColors.primary_400
	},
	deselected = {
		normal  = styleColors.panel_800,
		hovered = styleColors.panel_700,
		active  = styleColors.panel_600
	},
	selected = {
		normal = styleColors.primary_500,
		hovered = styleColors.primary_400,
		active = styleColors.primary_300
	},
	disabled = {
		normal = styleColors.panel_800,
		hovered = styleColors.panel_800,
		active = styleColors.panel_800
	},
	dark = {
		normal = styleColors.primary_900,
		hovered = styleColors.primary_700,
		active = styleColors.primary_600,
	},
	transparent = {
		normal = styleColors.transparent,
		hovered = styleColors.panel_700,
		active = styleColors.primary_600,
	},
	card = {
		normal = styleColors.primary_900,
		hovered = styleColors.primary_800,
		active = styleColors.primary_700
	},
	card_selected = {
		normal = styleColors.primary_800,
		hovered = styleColors.primary_700,
		active = styleColors.primary_600
	},
}

theme.colors = {
	reticuleCircle			= styleColors.gray_300,
	reticuleCircleDark		= styleColors.gray_400,
	frame					= styleColors.gray_300,
	frameDark				= styleColors.panel_500,
	navTarget				= styleColors.warning_100,
	navTargetDark			= styleColors.warning_300,
	combatTarget			= styleColors.danger_300,
	combatTargetDark		= styleColors.danger_500,

	radarBackground			= styleColors.panel_900:opacity(0.60),
	radarFrame				= styleColors.primary_500,

	navigationalElements	= styleColors.gray_300,
	deltaVCurrent			= styleColors.gray_400,
	deltaVManeuver			= styleColors.primary_300,
	deltaVRemaining			= styleColors.gray_100,
	deltaVTotal				= styleColors.panel_600:opacity(0.75),
	brakeBackground			= styleColors.panel_600:opacity(0.75),
	brakePrimary			= styleColors.gray_300,
	brakeSecondary			= styleColors.panel_500,
	brakeNow				= styleColors.success_500,
	brakeOvershoot			= styleColors.danger_500,
	maneuver				= styleColors.accent_300,
	maneuverDark			= styleColors.accent_500,
	mouseMovementDirection	= styleColors.accent_100,

	overlayWindowBg         = styleColors.panel_900:opacity(0.90),

	-- Foundational color styles. If an element doesn't have a specific
	-- semantic color, it should use one of these.
	uiPrimary				= styleColors.primary_700,
	uiPrimaryDark			= styleColors.primary_900,
	uiPrimaryLight			= styleColors.primary_500,
	uiBackground			= styleColors.panel_900,
	uiSurface				= styleColors.panel_800,
	uiError					= styleColors.danger_900,
	uiForeground			= styleColors.gray_500,

	unknown					= styleColors.unknown, -- used as an invalid color
	transparent				= styleColors.transparent,

	font					= styleColors.gray_100,
	fontDim					= styleColors.gray_400,
	fontDark				= styleColors.gray_600,

	-- FIXME: this color is primarily used to tint buttons by rendering over top of the frame color.
	-- This is atrocious for obvious reasons. Refactor button / frame rendering to draw an independent frame border.
	lightBlueBackground		= styleColors.primary_700:opacity(0.10),
	lightBlackBackground	= styleColors.panel_900:opacity(0.80), --black:opacity(0.40),
	windowBackground		= styleColors.panel_900,
	windowFrame				= styleColors.panel_700,
	notificationBackground	= styleColors.panel_800,
	modalBackground			= styleColors.panel_900,
	tableBackground			= styleColors.primary_900,
	tableHighlight			= styleColors.primary_800,
	tableSelection			= styleColors.primary_700,
	tableHighlightDisabled  = styleColors.panel_800,
	buttonBlue				= theme.buttonColors.selected.normal,
	buttonInk				= styleColors.white,

	-- ImGui theme default colors
	Text					= styleColors.gray_100,
	Button					= theme.buttonColors.default.normal,
	ButtonHovered			= theme.buttonColors.default.hovered,
	ButtonActive			= theme.buttonColors.default.active,
	CheckMark				= styleColors.primary_300a,
	PopupBg					= styleColors.panel_900,
	ModalWindowDimBg		= styleColors.black:opacity(0.35),
	FrameBg					= styleColors.panel_800,
	FrameBgHovered			= styleColors.panel_700,
	FrameBgActive			= styleColors.panel_700,

	Tab						= styleColors.primary_800,
	TabActive				= styleColors.primary_600,
	TabHovered				= styleColors.primary_500,

	Header					= styleColors.primary_800,
	HeaderHovered			= styleColors.primary_600,
	HeaderActive			= styleColors.primary_500,

	SliderGrab				= styleColors.primary_500a,
	SliderGrabActive		= styleColors.primary_300a,

	white					= styleColors.white,
	lightGrey				= styleColors.gray_300,
	grey					= styleColors.gray_500,
	darkGrey				= styleColors.panel_900,
	black					= styleColors.black,

	-- Semantic color names for specific parts of the UI with defined meaning

	alertYellow				= styleColors.warning_300,
	alertRed				= styleColors.danger_500,
	hyperspaceInfo			= styleColors.success_300,

	notificationInfo		= styleColors.gray_500,
	notificationGame		= styleColors.primary_500,
	notificationError		= styleColors.danger_700,

	econProfit				= styleColors.success_500,
	econLoss				= styleColors.danger_300,
	econMajorExport			= styleColors.success_300,
	econMinorExport			= styleColors.success_500,
	econMajorImport			= styleColors.accent_300,
	econMinorImport			= styleColors.accent_500,
	econIllegalCommodity	= styleColors.danger_300,

	gaugeCargo              = styleColors.gray_300,
	gaugeJettison           = styleColors.danger_500,

	gaugeBackground			= styleColors.panel_900:opacity(0.85),
	gaugePressure			= styleColors.primary_600,
	gaugeScanner			= styleColors.primary_500,
	gaugeTemperature		= styleColors.danger_500,
	gaugeShield				= styleColors.primary_300,
	gaugeHull				= styleColors.gray_200,
	gaugeWeapon				= styleColors.warning_300,
	gaugeVelocityLight		= styleColors.gray_100,
	gaugeVelocityDark		= styleColors.panel_800,
	gaugeThrustLight		= styleColors.gray_500,
	gaugeThrustDark			= styleColors.panel_900,
	gaugeEquipmentMarket	= styleColors.primary_600,

	radarCargo				= styleColors.primary_200,
	radarCloud				= styleColors.primary_200,
	radarCombatTarget		= styleColors.danger_500,
	radarMissile			= styleColors.danger_300,
	radarPlayerMissile		= styleColors.accent_500,
	radarNavTarget			= styleColors.success_500,
	radarShip				= styleColors.warning_500,
	radarStation			= styleColors.accent_300,
	radarUnknown			= styleColors.gray_500,

	systemMapGrid			= styleColors.panel_900,
	systemMapGridLeg		= styleColors.gray_600,
	systemMapObject			= styleColors.gray_300,
	systemMapPlanner		= styleColors.primary_500,
	systemMapPlannerOrbit	= styleColors.primary_500,
	systemMapPlayer			= styleColors.accent_400,
	systemMapPlayerOrbit	= styleColors.accent_400,
	systemMapShip			= styleColors.gray_300,
	systemMapShipOrbit		= styleColors.accent_700,
	systemMapSelectedShipOrbit = styleColors.warning_500,
	systemMapSystemBody		= styleColors.primary_100:shade(0.5),
	systemMapSystemBodyIcon	= styleColors.gray_300,
	systemMapSystemBodyOrbit = styleColors.success_500,
	systemMapLagrangePoint	= styleColors.accent_300,

	systemAtlasLabel        = styleColors.gray_300,
	systemAtlasLabelActive  = styleColors.gray_200,
	systemAtlasLine         = styleColors.primary_500,
	systemAtlasLineActive   = styleColors.primary_400,

	sectorMapLabelHighlight = styleColors.white:opacity(0.5),
	sectorMapLabelShade = styleColors.panel_800:opacity(0.9),

	equipScreenHighlight    = styleColors.gray_300,
	equipScreenBgText       = styleColors.gray_400,

	compareBetter = styleColors.accent_300,
	compareWorse  = styleColors.warning_300,
}

-- ImGui global theming styles
theme.styles = rescaleUI {
	WindowBorderSize = 0.0,
	WindowPadding = Vector2(8, 8),
	TabRounding = 0.0,
	TabPadding = Vector2(8, 6),
	ButtonPadding = Vector2(8, 6),
	ItemSpacing = Vector2(8, 6),
	ItemInnerSpacing = Vector2(4, 4),
	MainButtonSize = Vector2(38, 38),
	SmallButtonSize = Vector2(30, 30),
	IconButtonPadding = Vector2(3, 3),
	InlineIconPadding = Vector2(2, 2),
	MainButtonPadding = 3
}

theme.icons = {
	-- first row
	prograde = 0,
	retrograde = 1,
	radial_out = 2,
	radial_in = 3,
	antinormal = 4,
	normal = 5,
	frame = 6,
	maneuver = 7,
	forward = 8,
	backward = 9,
	down = 10,
	right = 11,
	up = 12,
	left = 13,
	bullseye = 14,
	square = 15,
	-- second row
	prograde_thin = 16,
	retrograde_thin = 17,
	radial_out_thin = 18,
	radial_in_thin = 19,
	antinormal_thin = 20,
	normal_thin = 21,
	frame_away = 22,
	direction = 24,
	direction_hollow = 25,
	direction_frame = 26,
	direction_frame_hollow = 27,
	direction_forward = 28,
	square_dashed = 29,
	empty = 30,
	semi_major_axis = 31,
	-- third row
	heavy_fighter = 32,
	medium_fighter = 33,
	light_fighter = 34,
	sun = 35,
	asteroid_hollow = 36,
	current_height = 37,
	current_periapsis = 38,
	-- UNUSED current_line = 39,
	current_apoapsis = 40,
	eta = 41,
	altitude = 42,
	gravity = 43,
	eccentricity = 44,
	inclination = 45,
	longitude = 46,
	latitude = 47,
	-- fourth row
	heavy_courier = 48,
	medium_courier = 49,
	light_courier = 50,
	rocky_planet = 51,
	ship = 52,
	landing_gear_up = 53,
	landing_gear_down = 54,
	ecm = 55,
	rotation_damping_on = 56,
	rotation_damping_off = 57,
	hyperspace = 58,
	hyperspace_off = 59,
	scanner = 60,
	message_bubble = 61,
	fuel = 63,
	-- fifth row
	heavy_passenger_shuttle = 64,
	medium_passenger_shuttle = 65,
	light_passenger_shuttle = 66,
	moon = 67,
	autopilot_set_speed = 68,
	autopilot_manual = 69,
	autopilot_fly_to = 70,
	autopilot_dock = 71,
	autopilot_hold = 72,
	autopilot_undock = 73,
	autopilot_undock_illegal = 74,
	autopilot_blastoff = 75,
	autopilot_blastoff_illegal = 76,
	autopilot_low_orbit = 77,
	autopilot_medium_orbit = 78,
	autopilot_high_orbit = 79,
	-- sixth row
	heavy_passenger_transport = 80,
	medium_passenger_transport = 81,
	light_passenger_transport = 82,
	gas_giant = 83,
	time_accel_stop = 84,
	time_accel_paused = 85,
	time_accel_1x = 86,
	time_accel_10x = 87,
	time_accel_100x = 88,
	time_accel_1000x = 89,
	time_accel_10000x = 90,
	pressure = 91,
	shield = 92,
	hull = 93,
	temperature = 94,
	rotate_view = 95,
	-- seventh row
	heavy_cargo_shuttle = 96,
	medium_cargo_shuttle = 97,
	light_cargo_shuttle = 98,
	spacestation = 99,
	time_backward_100x = 100,
	time_backward_10x = 101,
	time_backward_1x = 102,
	time_center = 103,
	time_forward_1x = 104,
	time_forward_10x = 105,
	time_forward_100x = 106,
	filter_bodies = 107,
	filter_stations = 108,
	filter_ships = 109,
	lagrange_marker = 110,
	system_overview_vertical = 111,
	-- eighth row
	heavy_freighter = 112,
	medium_freighter = 113,
	light_freighter = 114,
	starport = 115,
	warning_1 = 116,
	warning_2 = 117,
	warning_3 = 118,
	-- moon = 119, -- smaller duplicate of 67
	combattarget = 120,
	navtarget = 121,
	alert1 = 122,
	alert2 = 123,
	ecm_advanced = 124,
	systems_management = 125,
	distance = 126,
	filter = 127,
	-- ninth row
	view_internal = 128,
	view_external = 129,
	view_sidereal = 130,
	comms = 131,
	market = 132,
	bbs = 133,
	equipment = 134,
	repairs = 135,
	info = 136,
	personal_info = 137,
	personal = 138,
	roster = 139,
	map = 140,
	sector_map = 141,
	system_map = 142,
	system_overview = 143,
	-- tenth row
	galaxy_map = 144,
	settings = 145,
	language = 146,
	controls = 147,
	sound = 148,
	new = 149,
	skull = 150,
	mute = 151,
	unmute = 152,
	music = 153,
	zoom_in = 154,
	zoom_out = 155,
	search_lens = 156,
	message = 157,
	message_open = 158,
	search_binoculars = 159,
	-- eleventh row
	planet_grid = 160,
	bookmarks = 161,
	unlocked = 162,
	locked = 163,
	-- EMPTY = 164,
	label = 165,
	broadcast = 166,
	shield_other = 167,
	hud = 168,
	factory = 169,
	star = 170,
	delta = 171,
	clock = 172,
	orbit_prograde = 173,
	orbit_normal = 174,
	orbit_radial = 175,
	-- twelfth row
	-- BBS replacement icons
	-- TODO: mission display needs to be converted to use these instead of loading individual icons from disk
	-- mission_default = 176,
	alert_generic = 177,
	-- fuel_radioactive = 178,
	-- assassination = 179,
	money = 180,
	-- news = 181,
	-- crew = 182, -- duplicate of 138
	-- taxi = 183,
	-- taxi_urgent = 184,
	-- haul = 185,
	-- haul_urgent = 186,
	-- delivery = 187,
	-- delivery_urgent = 188,
	-- goodstrader = 189, -- duplicate of 132
	-- servicing_repair = 190,
	view_flyby = 191,
	-- thirteenth row
	cog = 192,
	gender = 193,
	nose = 194,
	mouth = 195,
	hair = 196,
	clothes = 197,
	accessories = 198,
	random = 199,
	periapsis = 200,
	apoapsis = 201,
	reset_view = 202,
	toggle_grid = 203,
	-- UNUSED plus = 204,
	-- EMPTY = 205
	decrease = 206,
	increase = 207,
	-- fourteenth row, wide icons
	missile_unguided = 208,
	missile_guided = 210,
	missile_smart = 212,
	missile_naval = 214,
	find_person = 216,
	cargo_manifest = 217,
	trashcan = 218,
	bookmark = 219,
	pencil = 220,
	fountain_pen = 221,
	cocktail_glass = 222,
	beer_glass = 223,
	-- fifteenth row
	chart = 224,
	binder = 225,
	-- navtarget = 226,		-- duplicate of 121
	-- ships_no_orbits = 227,	-- duplicate of 52
	ships_no_orbits = 52,
	ships_with_orbits = 228,
	lagrange_no_text = 229,
	lagrange_with_text = 230,
	route = 231,
	route_destination = 232,
	route_dist = 233,
	impact_warning = 234,
	econ_major_import = 235,
	econ_minor_import = 236,
	econ_major_export = 237,
	econ_minor_export = 238,
	cargo_crate = 239,
	-- sixteenth row
	econ_profit = 240,
	econ_loss = 241,
	starport_surface = 242,
	outpost_tiny = 243,
	outpost_small = 244,
	outpost_medium = 245,
	outpost_large = 246,
	station_orbital_large = 247,
	station_orbital_small = 248,
	station_observatory = 249,
	body_name = 250,
	body_day_length = 251,
	body_radius = 252,
	body_semi_major_axis = 253,
	body_orbit_period = 253,
	cargo_crate_illegal = 255,
	-- seventeenth row
	-- reticle icons 256..268
	follow_ori = 256,
	follow_pos = 257,
	follow_edge = 258,
	follow_fill = 259,
	manual_flight = 260,
	cruise_fwd = 261,
	cruise_up = 262,
	circ_manual_flight = 263,
	circ_cruise_fwd = 264,
	circ_cruise_up = 265,
	circ_clear_flwtarget = 266,
	speed_limiter = 267,
	deltav = 268,
	filesystem = 269,
	filesystem_save = 270,
	filesystem_load = 271,
	-- eighteenth row
	equip_cargo_scoop = 272,
	equip_fuel_scoop = 273,
	equip_multi_scoop = 274,
	equip_beamlaser = 275,
	equip_plasma_accelerator = 276,
	equip_pulsecannon = 277,
	equip_pulsecannon_rapid = 278,
	equip_mining_laser = 279,
	equip_dual_beamlaser = 280,
	equip_dual_plasma_accelerator = 281,
	equip_dual_pulsecannon = 282,
	equip_dual_pulsecannon_rapid = 283,
	equip_dual_mining_laser = 284,
	filesystem_delete = 285,
	case_sensitive = 286,
	delete_object = 287,
	-- nineteenth row
	equip_missile_unguided = 288,
	equip_missile_guided = 289,
	equip_missile_smart = 290,
	equip_missile_naval = 291,
	equip_shield_generator = 292,
	equip_atmo_shield_generator = 293,
	equip_scanner = 294,
	equip_radar = 295,
	equip_orbit_scanner = 296,
	equip_generic = 297,
	equip_cabin_empty = 298,
	equip_cabin_occupied = 299,
	equip_thrusters = 300,
	-- TODO: distinct icons for these
	equip_thrusters_basic = 300,
	equip_thrusters_medium = 300,
	equip_thrusters_best = 300,
	equip_trade_computer = 301,
	equip_autopilot = 302,
	equip_hyperdrive = 303,

	-- twentieth row
	plus = 304,
	minus = 305,
	cross = 306,
	decrease_max_thick = 307,
	decrease_thick = 308,
	increase_thick = 309,
	increase_max_thick = 310,
	decrease_min = 311,
	decrease_2 = 312,
	decrease_1 = 313,
	stop = 314,
	increase_1 = 315,
	increase_2 = 316,
	increase_max = 317,

	shipmarket_compare_better = 38,
	shipmarket_compare_worse = 40,

	circle_lg = 6,
	circle_md = 51,
	circle_sm = 110,

	-- TODO: manual / autopilot
	-- dummy, until actually defined correctly
	mouse_move_direction = 14,
}

return theme
