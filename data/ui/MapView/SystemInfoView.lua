local Engine = import("Engine")
local Game = import("Game")
local Lang = import("Lang")
local ui = Engine.ui
local l = Lang.GetResource("ui-core")
local TabView = import("ui/TabView")

local ICON_MAP = {
	BROWN_DWARF = 'icons/object_brown_dwarf.png',
	WHITE_DWARF = 'icons/object_white_dwarf.png',
	STAR_M = 'icons/object_star_m.png',
	STAR_K = 'icons/object_star_k.png',
	STAR_G = 'icons/object_star_g.png',
	STAR_F = 'icons/object_star_f.png',
	STAR_A = 'icons/object_star_a.png',
	STAR_B = 'icons/object_star_b.png',
	STAR_O = 'icons/object_star_b.png', -- shares B graphic for now
	STAR_M_GIANT = 'icons/object_star_m_giant.png',
	STAR_K_GIANT = 'icons/object_star_k_giant.png',
	STAR_G_GIANT = 'icons/object_star_g_giant.png',
	STAR_F_GIANT = 'icons/object_star_f_giant.png',
	STAR_A_GIANT = 'icons/object_star_a_giant.png',
	STAR_B_GIANT = 'icons/object_star_b_giant.png',
	STAR_O_GIANT = 'icons/object_star_o.png', -- uses old O type graphic
	STAR_M_SUPER_GIANT = 'icons/object_star_m_super_giant.png',
	STAR_K_SUPER_GIANT = 'icons/object_star_k_super_giant.png',
	STAR_G_SUPER_GIANT = 'icons/object_star_g_super_giant.png',
	STAR_F_SUPER_GIANT = 'icons/object_star_g_super_giant.png', -- shares G graphic for now
	STAR_A_SUPER_GIANT = 'icons/object_star_a_super_giant.png',
	STAR_B_SUPER_GIANT = 'icons/object_star_b_super_giant.png',
	STAR_O_SUPER_GIANT = 'icons/object_star_b_super_giant.png', -- uses B type graphic for now
	STAR_M_HYPER_GIANT = 'icons/object_star_m_hyper_giant.png',
	STAR_K_HYPER_GIANT = 'icons/object_star_k_hyper_giant.png',
	STAR_G_HYPER_GIANT = 'icons/object_star_g_hyper_giant.png',
	STAR_F_HYPER_GIANT = 'icons/object_star_f_hyper_giant.png',
	STAR_A_HYPER_GIANT = 'icons/object_star_a_hyper_giant.png',
	STAR_B_HYPER_GIANT = 'icons/object_star_b_hyper_giant.png',
	STAR_O_HYPER_GIANT = 'icons/object_star_b_hyper_giant.png', -- uses B type graphic for now
	STAR_M_WF = 'icons/object_star_m_wf.png',
	STAR_B_WF = 'icons/object_star_b_wf.png',
	STAR_O_WF = 'icons/object_star_o_wf.png',
	STAR_S_BH = 'icons/object_star_bh.png',
	STAR_IM_BH = 'icons/object_star_smbh.png',
	STAR_SM_BH = 'icons/object_star_smbh.png',
	PLANET_ASTEROID = 'icons/object_planet_asteroid.png',
	STARPORT_ORBITAL = 'icons/object_orbital_starport.png',
	PLANET_GAS_GIANT = function (body)
		local mass, averageTemp = body.mass, body.averageTemp
		local iconBase
		if mass > 1.8e27 then -- e.g., Jupiter
			iconBase = 'icons/object_planet_large_gas_giant'
		elseif mass > 4.5e26 then -- e.g., Saturn
			iconBase = 'icons/object_planet_medium_gas_giant'
		else -- e.g., Uranus, Neptune
			iconBase = 'icons/object_planet_small_gas_giant'
		end
		if averageTemp > 1000 then
			return iconBase .. '_hot.png'
		else
			return iconBase .. '.png'
		end
	end,
	PLANET_TERRESTRIAL = function (body)
		local mass, averageTemp = body.mass, body.averageTemp
		local volatileGas, volatileLiquid, volatileIces =
				body.volatileGas, body.volatileLiquid, body.volatileIces
		local volcanicity, atmosOxidizing = body.volcanicity, body.atmosOxidizing
		local life = body.life
		if volatileLiquid > 0.7 then
			if averageTemp > 250 then return 'icons/object_planet_water.png'
			else return 'icons/object_planet_ice.png' end
		elseif life > 0.9 and volatileGas > 0.6 then return 'icons/object_planet_life.png'
		elseif life > 0.8 and volatileGas > 0.5 then return 'icons/object_planet_life6.png'
		elseif life > 0.7 and volatileGas > 0.45 then return 'icons/object_planet_life7.png'
		elseif life > 0.6 and volatileGas > 0.4 then return 'icons/object_planet_life8.png'
		elseif life > 0.5 and volatileGas > 0.3 then return 'icons/object_planet_life4.png'
		elseif life > 0.4 and volatileGas > 0.2 then return 'icons/object_planet_life5.png'
		elseif life > 0.1 then
			if volatileGas > 0.2 then return 'icons/object_planet_life2.png'
			else return 'icons/object_planet_life3.png' end
		elseif mass < 6e23 then return 'icons/object_planet_small.png'
		elseif mass < 6e22 then return 'icons/object_planet_dwarf.png'
		elseif volatileLiquid < 0.1 and volatileGas > 0.2 then return 'icons/object_planet_desert.png'
		elseif volatileIces + volatileLiquid > 0.6 then
			if volatileIces > volatileLiquid or averageTemp < 250 then return 'icons/object_planet_ice.png'
			else return 'icons/object_planet_water.png' end
		elseif volatileGas > 0.5 then
			if atmosOxidizing > 0.5 then
				if averageTemp > 300 then
					return 'icons/object_planet_co2_2.png'
				elseif averageTemp > 250 then
					if volatileLiquid > 0.3 and volatileGas > 0.2 then
						return 'icons/object_planet_co2_4.png'
					else
						return 'icons/object_planet_co2_3.png'
					end
				else
					return 'icons/object_planet_co2.png'
				end
			else
				if averageTemp > 300 then return 'icons/object_planet_methane3.png'
				elseif averageTemp > 250 then return 'icons/object_planet_methane2.png'
				else return 'icons/object_planet_methane.png' end
			end
		elseif volatileLiquid > 0.1 and volatileGas < 0.1 then return 'icons/object_planet_ice.png'
		elseif volcanicity > 0.7 then return 'icons/object_planet_volcanic.png'
		else return 'icons/object_planet_small.png' end
	end,
}

local function pickIcon(body)
	local icon = ICON_MAP[body.type]
	if type(icon) == 'function' then
		icon = icon(body)
	end
	return icon
end

local GOVTYPE_DESCRIPTIONS = {
	['NONE']          = l.NO_CENTRAL_GOVERNANCE          ..' / '.. l.NO_ECONOMIC_ORDER,
	['EARTHCOLONIAL'] = l.EARTH_FEDERATION_COLONIAL_RULE ..' / '.. l.CAPITALISM,
	['EARTHDEMOC']    = l.EARTH_FEDERATION_DEMOCRACY     ..' / '.. l.CAPITALISM,
	['EMPIRERULE']    = l.IMPERIAL_RULE                  ..' / '.. l.PLANNED_ECONOMY,
	['CISLIBDEM']     = l.LIBERAL_DEMOCRACY              ..' / '.. l.CAPITALISM,
	['CISSOCDEM']     = l.SOCIAL_DEMOCRACY               ..' / '.. l.MIXED_ECONOMY,
	['LIBDEM']        = l.LIBERAL_DEMOCRACY              ..' / '.. l.CAPITALISM,
	['CORPORATE']     = l.CORPORATE_SYSTEM               ..' / '.. l.CAPITALISM,
	['SOCDEM']        = l.SOCIAL_DEMOCRACY               ..' / '.. l.MIXED_ECONOMY,
	['EARTHMILDICT']  = l.MILITARY_DICTATORSHIP          ..' / '.. l.CAPITALISM,
	['MILDICT1']      = l.MILITARY_DICTATORSHIP          ..' / '.. l.CAPITALISM,
	['MILDICT2']      = l.MILITARY_DICTATORSHIP          ..' / '.. l.MIXED_ECONOMY,
	['EMPIREMILDICT'] = l.MILITARY_DICTATORSHIP          ..' / '.. l.MIXED_ECONOMY,
	['COMMUNIST']     = l.COMMUNISM                      ..' / '.. l.PLANNED_ECONOMY,
	['PLUTOCRATIC']   = l.PLUTOCRATIC_DICTATORSHIP       ..' / '.. l.HARD_CAPITALISM,
	['DISORDER']      = l.VIOLENT_ANARCHY                ..' / '.. l.NO_ECONOMIC_ORDER,
}

local sysInfoWidgets = {
	title = ui:Label('SYSTEM NAME'):SetFont('HEADING_LARGE'),
	sector = ui:Label('[1, 2, 3]'),
	physicalDesc = ui:Label('Stable system with...'),
	desc = ui:MultiLineText('System description.'),
	shortDesc = ui:MultiLineText('Stable system with some stuff.'),
	govEcon = ui:Label('gov / econ'),
	allegiance = ui:Label('allegiance'),
	population = ui:Label('popluation'),
}

local bodyInfoWidgets = {
	shortDesc = ui:MultiLineText('{bodyname}: {shortDesc}'),
	propertyTable = ui:Table():SetColumnAlignment('LEFT'):SetColumnSpacing(20),
}

local function humanisePopulation(pop)
	if pop >= 1 then
		return string.format('Over %d billion', pop)
	elseif pop >= 1e-3 then
		return string.format('Over %d million', pop * 1000)
	elseif pop > 0 then
		return string.format('Only a few thousand')
	else
		return 'No registered inhabitants'
	end
end

local function humaniseMass(mass)
	local earth_mass = 5.9742e24
	local sol_mass = 1.98892e30
	if mass >= 2e28 then
		return string.interp(l.N_SOLAR_MASSES, {
			mass = string.format('%.2f', mass / sol_mass)
		})
	elseif mass >= 1e22 then
		return string.interp(l.N_EARTH_MASSES, {
			mass = string.format('%.2f', mass / earth_mass)
		})
	else
		return string.interp(l.N_TONNES, {
			mass = string.format('%.2e', mass / 1000)
		})
	end
end

local function humaniseRadius(radius)
	local earth_radius = 6378135.0
	local sol_radius = 6.955e8
	if radius >= 7e7 then
		return string.interp(l.N_SOLAR_RADII, {
			radius = string.format('%.1f', radius / sol_radius),
			radiuskm = string.format('%.0f', radius * 1e-3),
		})
	else
		return string.interp(l.N_EARTH_RADII, {
			radius = string.format('%.1f', radius / earth_radius),
			radiuskm = string.format('%.0f', radius * 1e-3),
		})
	end
end

local function humanisePeriod(period)
	local days = period / (60*60*24)
	if days >= 1000 then
		return string.interp(l.N_YEARS, {
			years = string.format('%.0f', days / 365.25),
		})
	elseif days >= 10 then
		return string.interp(l.N_DAYS, { days = string.format('%.0f', days) })
	elseif days >= 3 then
		return string.interp(l.N_DAYS, { days = string.format('%.1f', days) })
	else
		return string.interp(l.N_HOURS, {
			hours = string.format('%.0f', period/(60*60))
		})
	end
end

local function humaniseDistance(dist)
	local au = 149598000000.0
	if dist >= 0.1*au then
		return string.format('%.1f', dist / au) .. ' AU'
	elseif dist >= 10000 then
		return string.format('%.0f', dist / 1000) .. ' km'
	else
		return string.format('%.0f', dist) .. ' m'
	end
end

local function initSystemInfo(sys)
	sysInfoWidgets.title:SetText(sys.name)
	local path = sys.path
	sysInfoWidgets.sector:SetText(string.format(
		'[%d, %d, %d]', path.sectorX, path.sectorY, path.sectorZ))
	local nstations = sys:GetStationCount()
	local nsurface = 0
	local nbodies = sys:GetBodyCount() - nstations
	local quickInfo = string.interp(
		l.STABLE_SYSTEM_WITH_N_MAJOR_BODIES_STARPORTS, {
			['bodycount'] = nbodies,
			['body(s)'] = (nbodies == 1 and l.BODY or l.BODIES),
			['portcount'] = nstations,
			['starport(s)'] = (nstations == 1 and l.STARPORT or l.COUNT_STARPORTS),
		})
	if nstations > 0 then
		quickInfo = quickInfo .. string.interp(l.COUNT_ON_SURFACE, {
			['surfacecount'] = nsurface,
		})
	end
	sysInfoWidgets.physicalDesc:SetText(quickInfo)
	local desc = sys.description
	if desc == '' then
		desc = l.NO_SYSTEM_DESCRIPTION_FOUND
	end
	sysInfoWidgets.desc:SetText(desc)
	sysInfoWidgets.shortDesc:SetText(sys.shortDescription)
	sysInfoWidgets.govEcon:SetText(GOVTYPE_DESCRIPTIONS[sys.governmentType])
	sysInfoWidgets.allegiance:SetText(sys.faction.name)
	sysInfoWidgets.population:SetText(humanisePopulation(sys.population))
end

local function initBodyInfo(body)
	bodyInfoWidgets.shortDesc:SetText(body.name .. ': ' .. body.shortDescription)
	local ptable = bodyInfoWidgets.propertyTable
	ptable:ClearRows()

	-- Surface starports
	local children = body:GetChildren()
	local surface_ports = {}
	for i = 1, #children do
		if children[i].type == 'STARPORT_SURFACE' then
			surface_ports[#surface_ports + 1] = children[i].name
		end
	end
	if #surface_ports > 0 then
		ptable:AddRows({{l.STARPORTS, table.concat(surface_ports, ', ')},{"",""}})
	end

	-- Basic physical properties (mass, radius, rotational period)
	if body.superType ~= 'STARPORT' then
		ptable:AddRows({
			{l.MASS, humaniseMass(body.mass)},
			{l.RADIUS, humaniseRadius(body.radius)},
		})
	end
	if body.rotationPeriod > 0 then
		ptable:AddRow({
			l.ROTATIONAL_PERIOD, humanisePeriod(body.rotationPeriod * (60*60*24))
		})
	end

	-- More basic physical properties
	-- (surface temp & gravity, axial tilt, equatorial bulging)
	if body.superType ~= 'STARPORT' then
		ptable:AddRows({
			{"",""},
			{l.SURFACE_TEMPERATURE, string.interp(l.N_CELSIUS, {
				temperature = string.format('%.1f', body.averageTemp - 273),
			})},
			{l.SURFACE_GRAVITY, string.interp(l.N_M_PER_S_PER_S, {
				acceleration = string.format('%.2f', body.gravity),
			})},
		})
		if body.axialTilt ~= 0 then
			ptable:AddRow({
				l.AXIAL_TILT, string.interp(l.N_DEGREES, {
					angle = string.format('%.0f', body.axialTilt),
				})
			})
		end

		if body.aspectRatio ~= 1.0 then
			ptable:AddRow({
				l.EQUATORIAL_RADIUS_TO_POLAR_RADIUS_RATIO,
				string.format('%.2f', body.aspectRatio)
			})
		end
	end

	-- Orbital properties
	if body.parent ~= nil then
		ptable:AddRows({{"",""},{l.ORBITAL_PERIOD, humanisePeriod(body.orbitalPeriod)}})
		if body.eccentricity < 0.01 then
			ptable:AddRow({l.SEMI_MAJOR_AXIS, humaniseDistance(body.periapsis)})
		else
			ptable:AddRows({
				{l.PERIAPSIS_DISTANCE, humaniseDistance(body.periapsis)},
				{l.APOAPSIS_DISTANCE, humaniseDistance(body.apoapsis)},
				{l.ECCENTRICITY, string.format('%.2f', body.eccentricity)},
			})
		end
	end
end

local currentSystem
local systemInfoTab =
	ui:Scroller(ui:VBox(5):PackEnd({
		ui:HBox():PackEnd({
			sysInfoWidgets.title,
			ui:Expand('HORIZONTAL', ui:Align('RIGHT', sysInfoWidgets.sector)),
		}),
		sysInfoWidgets.shortDesc,
		"",
		ui:Label(l.QUICK_INFO):SetFont('HEADING_NORMAL'),
		sysInfoWidgets.physicalDesc,
		ui:Table():SetColumnAlignment('LEFT'):SetColumnSpacing(20):AddRows({
			{ui:Label(l.GOVERNMENT_AND_ECONOMY), sysInfoWidgets.govEcon},
			{ui:Label(l.ALLEGIANCE), sysInfoWidgets.allegiance},
			{ui:Label(l.POPULATION), sysInfoWidgets.population},
		}),
		"",
		ui:Label(l.NOTES):SetFont('HEADING_NORMAL'),
		sysInfoWidgets.desc,
	}))
local bodyInfoTab =
	ui:Scroller(ui:VBox(5):PackEnd({
		bodyInfoWidgets.shortDesc,
		"",
		bodyInfoWidgets.propertyTable,
	}))
local tradeInfoTab = ui:Label('Trade info goes here')

local infoTabs = TabView.New()
infoTabs:AddTab({
	id = 'System Info', title = l.SYSTEM_INFO, icon = 'Star',
	template = function () return systemInfoTab; end
})
infoTabs:AddTab({
	id = 'Body Info', title = l.BODY_INFO, icon = 'Planet',
	template = function () return bodyInfoTab; end
})
infoTabs:AddTab({
	id = 'Trade Info', title = l.TRADE_INFO, icon = 'ChartClipboard',
	template = function () return tradeInfoTab; end
})

local iconsContainer = ui:Scroller():SetFont('SMALL')
local sysInfoView =
	ui:Margin(4, 'ALL',
		ui:Grid({3,7}, 1)
			:SetCell(0,0, ui:Margin(2, 'RIGHT', ui:Background(iconsContainer)))
			:SetCell(1,0, ui:Margin(2, 'LEFT', ui:Background(infoTabs)))
	)

local currentBody, currentBodyIconSelector
local function handleClickBodyIcon(body, selector)
	if currentBody == body then return end
	initBodyInfo(body)
	if currentBodyIconSelector ~= nil then
		currentBodyIconSelector:SetShown(false)
	end
	selector:SetShown(true)
	currentBody = body
	currentBodyIconSelector = selector
	infoTabs:SwitchTo('Body Info')
end

local function bodyIconButton(body, onClick)
	local icon = pickIcon(body)
	if icon ~= nil then
		local selector = ui:SelectorBox('RECT',
			ui:Image(icon, {'PRESERVE_ASPECT'}):SetNaturalSize(1.25))
		selector:SetColor(0,0.9,0):SetShown(false)
		selector.onClick:Connect(function () onClick(body, selector); end)
		return selector
	else
		return nil
	end
end

local function buildMagicMoonLayout(body)
	local icon = bodyIconButton(body, handleClickBodyIcon)
	if icon == nil then return nil end
	local rows = { ui:Align('TOP', icon) }
	local children = body:GetChildren()
	for i = 1, #children do
		local icon = bodyIconButton(children[i], handleClickBodyIcon)
		if icon ~= nil then
			rows[#rows + 1] = ui:Align('TOP', icon)
		end
	end
	return ui:VBox(0):PackEnd(rows)
end

local function buildMagicPlanetLayout(body)
	local icon = bodyIconButton(body, handleClickBodyIcon)
	if icon == nil then return nil end
	local cols = {}
	local children = body:GetChildren()
	for i = 1, #children do
		local tree = buildMagicMoonLayout(children[i])
		if tree ~= nil then
			cols[#cols + 1] = tree
		end
	end

	local info =
		ui:VBox(1):PackEnd({
			ui:Align('TOP_LEFT', body.name),
			ui:HBox(0):PackEnd(cols)
		})
	return icon, info
end

local function buildMagicStarLayout(body)
	local stars
	if body.type == 'GRAVPOINT' then
		stars = body:GetChildren()
	else
		stars = { body }
	end
	local rows = {}
	for istar = 1, #stars do
		local star = stars[istar]
		local starTargetIcon = ui:Icon('Share')
		local starSelector =
			ui:SelectorBox('BRACKET', ui:Margin(3, 'HORIZONTAL',
				ui:Label(star.name):SetFont('HEADING_SMALL')))
		starSelector:SetShown(false)
		starSelector:SetColor(0.8, 0.8, 0.8)
		starTargetIcon:SetColor({r=0.6,g=0.6,b=0.6})
		local starTitleWithIcon = ui:HBox(5):PackEnd({
				ui:Align('MIDDLE', starTargetIcon:SetSize(12)),
				starSelector,
		})
		starTitleWithIcon.onClick:Connect(function ()
			local shown = not starSelector.shown
			local v = shown and 1 or 0.6
			starTargetIcon:SetColor({r=v,g=v,b=v})
			starSelector:SetShown(shown)
		end)

		rows[#rows + 1] = ui:Align('MIDDLE', starTitleWithIcon)
		rows[#rows + 1] = ui:Align('MIDDLE', bodyIconButton(star, handleClickBodyIcon))
		local children = star:GetChildren()
		if #children > 0 then
			local planetTable = ui:Table()
					:SetRowAlignment('CENTER')
					:SetColumnAlignment('LEFT')
					:SetColumnSpacing(3)
					:SetRowSpacing(0)
			for ichild = 1, #children do
				local icon, info = buildMagicPlanetLayout(children[ichild])
				if icon ~= nil then
					planetTable:AddRow({ui:Align('MIDDLE', icon), info})
				end
			end
			rows[#rows + 1] = planetTable
		end
	end
	return ui:Expand('HORIZONTAL', ui:VBox(3):PackEnd(rows))
end

ui.templates.SystemInfoView = function ()
	local sys = Game.player:GetHyperspaceTarget() or Game.system
	if sys:isa('SystemPath') then
		sys = sys:SystemOnly():GetStarSystem()
	end
	if sys ~= currentSystem then
		currentSystem = sys
		initSystemInfo(sys)
		iconsContainer:SetInnerWidget(buildMagicStarLayout(sys.rootBody))
	end
	return sysInfoView
end
