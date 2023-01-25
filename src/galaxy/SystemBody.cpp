// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemBody.h"

#include "AtmosphereParameters.h"
#include "EnumStrings.h"
#include "Game.h"
#include "Lang.h"
#include "Pi.h"
#include "enum_table.h"
#include "utils.h"

SystemBody::SystemBody(const SystemPath &path, StarSystem *system) :
	m_parent(nullptr),
	m_path(path),
	m_seed(0),
	m_aspectRatio(1, 1),
	m_orbMin(0),
	m_orbMax(0),
	m_rotationalPhaseAtStart(0),
	m_semiMajorAxis(0),
	m_eccentricity(0),
	m_orbitalOffset(0),
	m_axialTilt(0),
	m_inclination(0),
	m_averageTemp(0),
	m_type(TYPE_GRAVPOINT),
	m_isCustomBody(false),
	m_heightMapFractal(0),
	m_atmosDensity(0.0),
	m_system(system)
{
}

bool SystemBody::HasAtmosphere() const
{
	return (m_volatileGas > fixed(1, 100));
}

bool SystemBody::IsScoopable() const
{
	if (GetSuperType() == SUPERTYPE_GAS_GIANT)
		return true;
	if ((m_type == TYPE_PLANET_TERRESTRIAL) &&
		m_volatileGas > fixed(1, 100) &&
		m_atmosOxidizing > fixed(3, 10) &&
		m_atmosOxidizing <= fixed(55, 100))
		return true;
	return false;
}

// Calculate parameters used in the atmospheric model for shaders
AtmosphereParameters SystemBody::CalcAtmosphereParams() const
{
	PROFILE_SCOPED()
	AtmosphereParameters params;

	double atmosDensity;

	GetAtmosphereFlavor(&params.atmosCol, &atmosDensity);
	// adjust global atmosphere opacity
	atmosDensity *= 1e-5;

	params.atmosDensity = static_cast<float>(atmosDensity);

	// Calculate parameters used in the atmospheric model for shaders
	// Isothermal atmospheric model
	// See http://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_atmospheric_pressure_variation
	// This model features an exponential decrease in pressure and density with altitude.
	// The scale height is 1/the exponential coefficient.

	// The equation for pressure is:
	// Pressure at height h = Pressure surface * e^((-Mg/RT)*h)

	// calculate (inverse) atmosphere scale height
	// The formula for scale height is:
	// h = RT / Mg
	// h is height above the surface in meters
	// R is the universal gas constant
	// T is the surface temperature in Kelvin
	// g is the gravity in m/s^2
	// M is the molar mass of air in kg/mol

	// calculate gravity
	// radius of the planet
	const double radiusPlanet_in_m = (m_radius.ToDouble() * EARTH_RADIUS);
	const double massPlanet_in_kg = (m_mass.ToDouble() * EARTH_MASS);
	const double g = G * massPlanet_in_kg / (radiusPlanet_in_m * radiusPlanet_in_m);

	double T = static_cast<double>(m_averageTemp);

	// XXX hack to avoid issues with sysgen giving 0 temps
	// temporary as part of sysgen needs to be rewritten before the proper fix can be used
	if (T < 1)
		T = 165;

	// We have two kinds of atmosphere: Earth-like and gas giant (hydrogen/helium)
	const double M = m_type == TYPE_PLANET_GAS_GIANT ? 0.0023139903 : 0.02897f; // in kg/mol

	float atmosScaleHeight = static_cast<float>(GAS_CONSTANT_R * T / (M * g));

	// min of 2.0 corresponds to a scale height of 1/20 of the planet's radius,
	params.atmosInvScaleHeight = std::max(20.0f, static_cast<float>(GetRadius() / atmosScaleHeight));
	// integrate atmospheric density between surface and this radius. this is 10x the scale
	// height, which should be a height at which the atmospheric density is negligible
	params.atmosRadius = 1.0f + static_cast<float>(10.0f * atmosScaleHeight) / GetRadius();

	params.planetRadius = static_cast<float>(radiusPlanet_in_m);

	return params;
}

SystemBody::BodySuperType SystemBody::GetSuperType() const
{
	switch (m_type) {
	case TYPE_BROWN_DWARF:
	case TYPE_WHITE_DWARF:
	case TYPE_STAR_M:
	case TYPE_STAR_K:
	case TYPE_STAR_G:
	case TYPE_STAR_F:
	case TYPE_STAR_A:
	case TYPE_STAR_B:
	case TYPE_STAR_O:
	case TYPE_STAR_M_GIANT:
	case TYPE_STAR_K_GIANT:
	case TYPE_STAR_G_GIANT:
	case TYPE_STAR_F_GIANT:
	case TYPE_STAR_A_GIANT:
	case TYPE_STAR_B_GIANT:
	case TYPE_STAR_O_GIANT:
	case TYPE_STAR_M_SUPER_GIANT:
	case TYPE_STAR_K_SUPER_GIANT:
	case TYPE_STAR_G_SUPER_GIANT:
	case TYPE_STAR_F_SUPER_GIANT:
	case TYPE_STAR_A_SUPER_GIANT:
	case TYPE_STAR_B_SUPER_GIANT:
	case TYPE_STAR_O_SUPER_GIANT:
	case TYPE_STAR_M_HYPER_GIANT:
	case TYPE_STAR_K_HYPER_GIANT:
	case TYPE_STAR_G_HYPER_GIANT:
	case TYPE_STAR_F_HYPER_GIANT:
	case TYPE_STAR_A_HYPER_GIANT:
	case TYPE_STAR_B_HYPER_GIANT:
	case TYPE_STAR_O_HYPER_GIANT:
	case TYPE_STAR_M_WF:
	case TYPE_STAR_B_WF:
	case TYPE_STAR_O_WF:
	case TYPE_STAR_S_BH:
	case TYPE_STAR_IM_BH:
	case TYPE_STAR_SM_BH:
		return SUPERTYPE_STAR;
	case TYPE_PLANET_GAS_GIANT:
		return SUPERTYPE_GAS_GIANT;
	case TYPE_PLANET_ASTEROID:
	case TYPE_PLANET_TERRESTRIAL:
		return SUPERTYPE_ROCKY_PLANET;
	case TYPE_STARPORT_ORBITAL:
	case TYPE_STARPORT_SURFACE:
		return SUPERTYPE_STARPORT;
	case TYPE_GRAVPOINT:
		return SUPERTYPE_NONE;
	default:
		Output("Warning: Invalid SuperBody Type found.\n");
		return SUPERTYPE_NONE;
	}
}

std::string SystemBody::GetAstroDescription() const
{
	PROFILE_SCOPED()
	switch (m_type) {
	case TYPE_BROWN_DWARF: return Lang::BROWN_DWARF;
	case TYPE_WHITE_DWARF: return Lang::WHITE_DWARF;
	case TYPE_STAR_M: return Lang::STAR_M;
	case TYPE_STAR_K: return Lang::STAR_K;
	case TYPE_STAR_G: return Lang::STAR_G;
	case TYPE_STAR_F: return Lang::STAR_F;
	case TYPE_STAR_A: return Lang::STAR_A;
	case TYPE_STAR_B: return Lang::STAR_B;
	case TYPE_STAR_O: return Lang::STAR_O;
	case TYPE_STAR_M_GIANT: return Lang::STAR_M_GIANT;
	case TYPE_STAR_K_GIANT: return Lang::STAR_K_GIANT;
	case TYPE_STAR_G_GIANT: return Lang::STAR_G_GIANT;
	case TYPE_STAR_F_GIANT: return Lang::STAR_AF_GIANT;
	case TYPE_STAR_A_GIANT: return Lang::STAR_AF_GIANT;
	case TYPE_STAR_B_GIANT: return Lang::STAR_B_GIANT;
	case TYPE_STAR_O_GIANT: return Lang::STAR_O_GIANT;
	case TYPE_STAR_M_SUPER_GIANT: return Lang::STAR_M_SUPER_GIANT;
	case TYPE_STAR_K_SUPER_GIANT: return Lang::STAR_K_SUPER_GIANT;
	case TYPE_STAR_G_SUPER_GIANT: return Lang::STAR_G_SUPER_GIANT;
	case TYPE_STAR_F_SUPER_GIANT: return Lang::STAR_AF_SUPER_GIANT;
	case TYPE_STAR_A_SUPER_GIANT: return Lang::STAR_AF_SUPER_GIANT;
	case TYPE_STAR_B_SUPER_GIANT: return Lang::STAR_B_SUPER_GIANT;
	case TYPE_STAR_O_SUPER_GIANT: return Lang::STAR_O_SUPER_GIANT;
	case TYPE_STAR_M_HYPER_GIANT: return Lang::STAR_M_HYPER_GIANT;
	case TYPE_STAR_K_HYPER_GIANT: return Lang::STAR_K_HYPER_GIANT;
	case TYPE_STAR_G_HYPER_GIANT: return Lang::STAR_G_HYPER_GIANT;
	case TYPE_STAR_F_HYPER_GIANT: return Lang::STAR_AF_HYPER_GIANT;
	case TYPE_STAR_A_HYPER_GIANT: return Lang::STAR_AF_HYPER_GIANT;
	case TYPE_STAR_B_HYPER_GIANT: return Lang::STAR_B_HYPER_GIANT;
	case TYPE_STAR_O_HYPER_GIANT: return Lang::STAR_O_HYPER_GIANT;
	case TYPE_STAR_M_WF: return Lang::STAR_M_WF;
	case TYPE_STAR_B_WF: return Lang::STAR_B_WF;
	case TYPE_STAR_O_WF: return Lang::STAR_O_WF;
	case TYPE_STAR_S_BH: return Lang::STAR_S_BH;
	case TYPE_STAR_IM_BH: return Lang::STAR_IM_BH;
	case TYPE_STAR_SM_BH: return Lang::STAR_SM_BH;
	case TYPE_PLANET_GAS_GIANT:
		if (m_mass > 800) return Lang::VERY_LARGE_GAS_GIANT;
		if (m_mass > 300) return Lang::LARGE_GAS_GIANT;
		if (m_mass > 80)
			return Lang::MEDIUM_GAS_GIANT;
		else
			return Lang::SMALL_GAS_GIANT;
	case TYPE_PLANET_ASTEROID:
	case TYPE_PLANET_TERRESTRIAL: {
		std::string s;

		// Is it a moon? or a dwarf planet?
		bool dwarfplanet = false;
		bool moon = false;
		if (m_parent && (m_parent->GetType() == TYPE_PLANET_TERRESTRIAL || m_parent->GetType() == TYPE_PLANET_GAS_GIANT))
			moon = true;

		// Is it an asteroid or a tiny moon?
		if (m_type == TYPE_PLANET_ASTEROID) {
			if (moon) {
				s += Lang::TINY;
				s += Lang::ROCKY_MOON;
				return s;
			}
			else return Lang::ASTEROID;
		}

		if (m_mass > fixed(2, 1))
			s = Lang::MASSIVE;
		else if (m_mass > fixed(3, 2))
			s = Lang::LARGE;
		else if (m_mass > fixed(1, 2))
			s = Lang::MEDIUM;
		else if (m_mass > fixed(12,1000))		// ~Weight of the moon (By definition here to be
			s = Lang::SMALL;					// the upper limit of a Dwarf planet
		else if (m_mass > fixed(1, 12000)) {	// ~Larger than the weight of Salacia (0.7% moon mass)
			if (moon) {							// which is considered not a dwarf planet
				s = Lang::TINY;
			} else {
				dwarfplanet = true;
			}
		} else {
			if (moon) {
				s = Lang::TINY;
			} else {
				return Lang::ASTEROID;
			}
		}

		if (m_volcanicity > fixed(7, 10)) {
			if (s.size())
				s += Lang::COMMA_HIGHLY_VOLCANIC;
			else
				s = Lang::HIGHLY_VOLCANIC;
		}

		// m_averageTemp <-- in degrees Kelvin. -273 for Celsius.
		// ^--- is not in fixed() format, but rather plain integer

		if (m_volatileIces + m_volatileLiquid > fixed(4, 5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (moon) {
					s += (m_averageTemp < 273) ? Lang::ICE_MOON: Lang::ROCKY_MOON;
				} else if (dwarfplanet) {
					s += (m_averageTemp < 273) ? Lang::ICE_DWARF_PLANET : Lang::DWARF_PLANET_TERRESTRIAL;
				} else {
					s += (m_averageTemp < 273) ? Lang::ICE_WORLD : Lang::ROCKY_PLANET;
				}
			} else {
				if (moon) {
					s += (m_averageTemp < 273) ? Lang::ICE_MOON : Lang::OCEANICMOON;
				} else if (dwarfplanet) {
					s += (m_averageTemp < 273) ? Lang::DWARF_PLANET_MOSTLY_COVERED_IN_ICE : Lang::DWARF_PLANET_CONTAINING_LIQUID_WATER;
				} else {
					s += (m_averageTemp < 273) ? Lang::ICE_WORLD : Lang::OCEANICWORLD;
				}
			}
			// what is a waterworld with temperature above 100C? possible?
		} else if (m_volatileLiquid > fixed(2, 5) && moon) {
			s += (m_averageTemp > 273) ? Lang::MOON_CONTAINING_LIQUID_WATER : Lang::MOON_WITH_SOME_ICE;
		} else if (m_volatileLiquid > fixed(2, 5)) {
			s += (m_averageTemp > 273) ? Lang::PLANET_CONTAINING_LIQUID_WATER : Lang::PLANET_WITH_SOME_ICE;
		} else if (moon) {
			s += (m_volatileLiquid > fixed(1, 5)) ? Lang::ROCKY_MOON_CONTAINING_SOME_LIQUIDS : Lang::ROCKY_MOON;
		} else if (dwarfplanet) {
			s += (m_volatileLiquid > fixed(1, 5)) ? Lang::DWARF_PLANET_CONTAINING_SOME_LIQUIDS : Lang::DWARF_PLANET_TERRESTRIAL;
		} else {
			s += (m_volatileLiquid > fixed(1, 5)) ? Lang::ROCKY_PLANET_CONTAINING_COME_LIQUIDS : Lang::ROCKY_PLANET;
		}

		if (m_volatileGas < fixed(1, 100)) {
			s += Lang::WITH_NO_SIGNIFICANT_ATMOSPHERE;
		} else {
			bool article = false;
			std::string thickness;
			if (m_volatileGas < fixed(1, 10))
				thickness = Lang::TENUOUS;
			else if (m_volatileGas < fixed(1, 5))
				thickness = Lang::THIN;
			else if (m_volatileGas < fixed(2, 1)) // normal atmosphere
				article = true;
			else if (m_volatileGas < fixed(20, 1))
				thickness = Lang::THICK;
			else
				thickness = Lang::VERY_DENSE;

			if (m_atmosOxidizing > fixed(95, 100)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::AN_O2_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::O2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(7, 10)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_CO2_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::CO2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(65, 100)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_CO_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::CO_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(55, 100)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_CH4_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::CH4_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(3, 10)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_H_ATMOSPHERE; // IsScoopable depends on these if/then/} else values fixed(3,10) -> fixed(55,100) == hydrogen
				} else
					s += Lang::WITH + thickness + Lang::H_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(2, 10)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_HE_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::HE_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(15, 100)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::AN_AR_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::AR_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(1, 10)) {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_S_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::S_ATMOSPHERE;
			} else {
				if  (article) {
					s += Lang::WITH + thickness + Lang::A_N_ATMOSPHERE;
				} else
					s += Lang::WITH + thickness + Lang::N_ATMOSPHERE;
			}
		}

		if (m_life > fixed(1, 2)) {
			s += Lang::AND_HIGHLY_COMPLEX_ECOSYSTEM;
		} else if (m_life > fixed(1, 10)) {
			s += Lang::AND_INDIGENOUS_PLANT_LIFE;
		} else if (m_life > fixed()) {
			s += Lang::AND_INDIGENOUS_MICROBIAL_LIFE;
		} else {
			s += ".";
		}

		s[0] = std::toupper(s[0]);

		return s;
	}
	case TYPE_STARPORT_ORBITAL:
		return Lang::ORBITAL_STARPORT;
	case TYPE_STARPORT_SURFACE:
		return Lang::STARPORT;
	case TYPE_GRAVPOINT:
	default:
		Output("Warning: Invalid Astro Body Description found.\n");
		return Lang::UNKNOWN;
	}
}

const char *SystemBody::GetIcon() const
{
	PROFILE_SCOPED()
	switch (m_type) {
	case TYPE_BROWN_DWARF: return "icons/object_brown_dwarf.png";
	case TYPE_WHITE_DWARF: return "icons/object_white_dwarf.png";
	case TYPE_STAR_M: return "icons/object_star_m.png";
	case TYPE_STAR_K: return "icons/object_star_k.png";
	case TYPE_STAR_G: return "icons/object_star_g.png";
	case TYPE_STAR_F: return "icons/object_star_f.png";
	case TYPE_STAR_A: return "icons/object_star_a.png";
	case TYPE_STAR_B: return "icons/object_star_b.png";
	case TYPE_STAR_O: return "icons/object_star_b.png"; //shares B graphic for now
	case TYPE_STAR_M_GIANT: return "icons/object_star_m_giant.png";
	case TYPE_STAR_K_GIANT: return "icons/object_star_k_giant.png";
	case TYPE_STAR_G_GIANT: return "icons/object_star_g_giant.png";
	case TYPE_STAR_F_GIANT: return "icons/object_star_f_giant.png";
	case TYPE_STAR_A_GIANT: return "icons/object_star_a_giant.png";
	case TYPE_STAR_B_GIANT: return "icons/object_star_b_giant.png";
	case TYPE_STAR_O_GIANT: return "icons/object_star_o.png"; // uses old O type graphic
	case TYPE_STAR_M_SUPER_GIANT: return "icons/object_star_m_super_giant.png";
	case TYPE_STAR_K_SUPER_GIANT: return "icons/object_star_k_super_giant.png";
	case TYPE_STAR_G_SUPER_GIANT: return "icons/object_star_g_super_giant.png";
	case TYPE_STAR_F_SUPER_GIANT: return "icons/object_star_g_super_giant.png"; //shares G graphic for now
	case TYPE_STAR_A_SUPER_GIANT: return "icons/object_star_a_super_giant.png";
	case TYPE_STAR_B_SUPER_GIANT: return "icons/object_star_b_super_giant.png";
	case TYPE_STAR_O_SUPER_GIANT: return "icons/object_star_b_super_giant.png"; // uses B type graphic for now
	case TYPE_STAR_M_HYPER_GIANT: return "icons/object_star_m_hyper_giant.png";
	case TYPE_STAR_K_HYPER_GIANT: return "icons/object_star_k_hyper_giant.png";
	case TYPE_STAR_G_HYPER_GIANT: return "icons/object_star_g_hyper_giant.png";
	case TYPE_STAR_F_HYPER_GIANT: return "icons/object_star_f_hyper_giant.png";
	case TYPE_STAR_A_HYPER_GIANT: return "icons/object_star_a_hyper_giant.png";
	case TYPE_STAR_B_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png";
	case TYPE_STAR_O_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png"; // uses B type graphic for now
	case TYPE_STAR_M_WF: return "icons/object_star_m_wf.png";
	case TYPE_STAR_B_WF: return "icons/object_star_b_wf.png";
	case TYPE_STAR_O_WF: return "icons/object_star_o_wf.png";
	case TYPE_STAR_S_BH: return "icons/object_star_bh.png";
	case TYPE_STAR_IM_BH: return "icons/object_star_smbh.png";
	case TYPE_STAR_SM_BH: return "icons/object_star_smbh.png";
	case TYPE_PLANET_GAS_GIANT:
		if (m_mass > 800) {
			if (m_averageTemp > 1000)
				return "icons/object_planet_large_gas_giant_hot.png";
			else
				return "icons/object_planet_large_gas_giant.png";
		}
		if (m_mass > 300) {
			if (m_averageTemp > 1000)
				return "icons/object_planet_large_gas_giant_hot.png";
			else
				return "icons/object_planet_large_gas_giant.png";
		}
		if (m_mass > 80) {
			if (m_averageTemp > 1000)
				return "icons/object_planet_medium_gas_giant_hot.png";
			else
				return "icons/object_planet_medium_gas_giant.png";
		} else {
			if (m_averageTemp > 1000)
				return "icons/object_planet_small_gas_giant_hot.png";
			else
				return "icons/object_planet_small_gas_giant.png";
		}
	case TYPE_PLANET_ASTEROID:
		return "icons/object_planet_asteroid.png";
	case TYPE_PLANET_TERRESTRIAL:
		if (m_volatileLiquid > fixed(7, 10)) {
			if (m_averageTemp > 250)
				return "icons/object_planet_water.png";
			else
				return "icons/object_planet_ice.png";
		}
		if ((m_life > fixed(9, 10)) &&
			(m_volatileGas > fixed(6, 10))) return "icons/object_planet_life.png";
		if ((m_life > fixed(8, 10)) &&
			(m_volatileGas > fixed(5, 10))) return "icons/object_planet_life6.png";
		if ((m_life > fixed(7, 10)) &&
			(m_volatileGas > fixed(45, 100))) return "icons/object_planet_life7.png";
		if ((m_life > fixed(6, 10)) &&
			(m_volatileGas > fixed(4, 10))) return "icons/object_planet_life8.png";
		if ((m_life > fixed(5, 10)) &&
			(m_volatileGas > fixed(3, 10))) return "icons/object_planet_life4.png";
		if ((m_life > fixed(4, 10)) &&
			(m_volatileGas > fixed(2, 10))) return "icons/object_planet_life5.png";
		if ((m_life > fixed(1, 10)) &&
			(m_volatileGas > fixed(2, 10))) return "icons/object_planet_life2.png";
		if (m_life > fixed(1, 10)) return "icons/object_planet_life3.png";
		if (m_mass < fixed(1, 100)) return "icons/object_planet_dwarf.png";
		if (m_mass < fixed(1, 10)) return "icons/object_planet_small.png";
		if ((m_volatileLiquid < fixed(1, 10)) &&
			(m_volatileGas > fixed(1, 5))) return "icons/object_planet_desert.png";

		if (m_volatileIces + m_volatileLiquid > fixed(3, 5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (m_averageTemp < 250) return "icons/object_planet_ice.png";
			} else {
				if (m_averageTemp > 250) {
					return "icons/object_planet_water.png";
				} else
					return "icons/object_planet_ice.png";
			}
		}

		if (m_volatileGas > fixed(1, 2)) {
			if (m_atmosOxidizing < fixed(1, 2)) {
				if (m_averageTemp > 300)
					return "icons/object_planet_methane3.png";
				else if (m_averageTemp > 250)
					return "icons/object_planet_methane2.png";
				else
					return "icons/object_planet_methane.png";
			} else {
				if (m_averageTemp > 300)
					return "icons/object_planet_co2_2.png";
				else if (m_averageTemp > 250) {
					if ((m_volatileLiquid > fixed(3, 10)) && (m_volatileGas > fixed(2, 10)))
						return "icons/object_planet_co2_4.png";
					else
						return "icons/object_planet_co2_3.png";
				} else
					return "icons/object_planet_co2.png";
			}
		}

		if ((m_volatileLiquid > fixed(1, 10)) &&
			(m_volatileGas < fixed(1, 10))) return "icons/object_planet_ice.png";
		if (m_volcanicity > fixed(7, 10)) return "icons/object_planet_volcanic.png";
		return "icons/object_planet_small.png";
		/*
      "icons/object_planet_water_n1.png"
      "icons/object_planet_life3.png"
      "icons/object_planet_life2.png"
      */
	case TYPE_STARPORT_ORBITAL:
		return "icons/object_orbital_starport.png";
	case TYPE_GRAVPOINT:
	case TYPE_STARPORT_SURFACE:
	default:
		Output("Warning: Invalid body icon.\n");
		return 0;
	}
}

bool SystemBody::IsPlanet() const
{
	BodySuperType st = GetSuperType();
	if (st != BodySuperType::SUPERTYPE_ROCKY_PLANET && st != BodySuperType::SUPERTYPE_GAS_GIANT)
		return false;
	SystemBody *p = GetParent();
	if (p != nullptr && p->GetSuperType() == BodySuperType::SUPERTYPE_STAR) {
		return true;
	} else {
		return false;
	}
}

void CollectSystemBodies(SystemBody *sb, std::vector<SystemBody *> &sb_vector)
{
	for (SystemBody *body : sb->GetChildren()) {
		sb_vector.push_back(body);
		if (sb->HasChildren()) CollectSystemBodies(body, sb_vector);
	}
}

const std::vector<SystemBody *> SystemBody::CollectAllChildren()
{
	std::vector<SystemBody *> sb_vector;
	// At least avoid initial reallocations
	sb_vector.reserve(m_children.size());

	for (SystemBody *sbody : m_children) {
		sb_vector.push_back(sbody);
		if (HasChildren()) CollectSystemBodies(this, sb_vector);
	}
	return sb_vector;
}

double SystemBody::GetMaxChildOrbitalDistance() const
{
	PROFILE_SCOPED()
	double max = 0;
	for (unsigned int i = 0; i < m_children.size(); i++) {
		if (m_children[i]->m_orbMax.ToDouble() > max) {
			max = m_children[i]->m_orbMax.ToDouble();
		}
	}
	return AU * max;
}

bool SystemBody::IsCoOrbitalWith(const SystemBody *other) const
{
	if (m_parent && m_parent->GetType() == SystemBody::TYPE_GRAVPOINT && ((m_parent->m_children[0] == this && m_parent->m_children[1] == other) || (m_parent->m_children[1] == this && m_parent->m_children[0] == other)))
		return true;
	return false;
}

bool SystemBody::IsCoOrbital() const
{
	if (m_parent && m_parent->GetType() == SystemBody::TYPE_GRAVPOINT && (m_parent->m_children[0] == this || m_parent->m_children[1] == this))
		return true;
	return false;
}

double SystemBody::CalcSurfaceGravity() const
{
	double r = GetRadius();
	if (r > 0.0) {
		return G * GetMass() / pow(r, 2);
	} else {
		return 0.0;
	}
}

void SystemBody::Dump(FILE *file, const char *indent) const
{
	fprintf(file, "%sSystemBody(%d,%d,%d,%u,%u) : %s/%s %s{\n", indent, m_path.sectorX, m_path.sectorY, m_path.sectorZ, m_path.systemIndex,
		m_path.bodyIndex, EnumStrings::GetString("BodySuperType", GetSuperType()), EnumStrings::GetString("BodyType", m_type),
		m_isCustomBody ? "CUSTOM " : "");
	fprintf(file, "%s\t\"%s\"\n", indent, m_name.c_str());
	fprintf(file, "%s\tmass %.6f\n", indent, m_mass.ToDouble());
	fprintf(file, "%s\torbit a=%.6f, e=%.6f, phase=%.6f\n", indent, m_orbit.GetSemiMajorAxis(), m_orbit.GetEccentricity(),
		m_orbit.GetOrbitalPhaseAtStart());
	fprintf(file, "%s\torbit a=%.6f, e=%.6f, orbMin=%.6f, orbMax=%.6f\n", indent, m_semiMajorAxis.ToDouble(), m_eccentricity.ToDouble(),
		m_orbMin.ToDouble(), m_orbMax.ToDouble());
	fprintf(file, "%s\t\toffset=%.6f, phase=%.6f, inclination=%.6f\n", indent, m_orbitalOffset.ToDouble(), m_orbitalPhaseAtStart.ToDouble(),
		m_inclination.ToDouble());
	if (m_type != TYPE_GRAVPOINT) {
		fprintf(file, "%s\tseed %u\n", indent, m_seed);
		fprintf(file, "%s\tradius %.6f, aspect %.6f\n", indent, m_radius.ToDouble(), m_aspectRatio.ToDouble());
		fprintf(file, "%s\taxial tilt %.6f, period %.6f, phase %.6f\n", indent, m_axialTilt.ToDouble(), m_rotationPeriod.ToDouble(),
			m_rotationalPhaseAtStart.ToDouble());
		fprintf(file, "%s\ttemperature %d\n", indent, m_averageTemp);
		fprintf(file, "%s\tmetalicity %.2f, volcanicity %.2f\n", indent, m_metallicity.ToDouble() * 100.0, m_volcanicity.ToDouble() * 100.0);
		fprintf(file, "%s\tvolatiles gas=%.2f, liquid=%.2f, ice=%.2f\n", indent, m_volatileGas.ToDouble() * 100.0,
			m_volatileLiquid.ToDouble() * 100.0, m_volatileIces.ToDouble() * 100.0);
		fprintf(file, "%s\tlife %.2f\n", indent, m_life.ToDouble() * 100.0);
		fprintf(file, "%s\tatmosphere oxidizing=%.2f, color=(%hhu,%hhu,%hhu,%hhu), density=%.6f\n", indent,
			m_atmosOxidizing.ToDouble() * 100.0, m_atmosColor.r, m_atmosColor.g, m_atmosColor.b, m_atmosColor.a, m_atmosDensity);
		fprintf(file, "%s\trings minRadius=%.2f, maxRadius=%.2f, color=(%hhu,%hhu,%hhu,%hhu)\n", indent, m_rings.minRadius.ToDouble() * 100.0,
			m_rings.maxRadius.ToDouble() * 100.0, m_rings.baseColor.r, m_rings.baseColor.g, m_rings.baseColor.b, m_rings.baseColor.a);
		fprintf(file, "%s\thuman activity %.2f, population %.0f, agricultural %.2f\n", indent, m_humanActivity.ToDouble() * 100.0,
			m_population.ToDouble() * 1e9, m_agricultural.ToDouble() * 100.0);
		if (!m_heightMapFilename.empty()) {
			fprintf(file, "%s\theightmap \"%s\", fractal %u\n", indent, m_heightMapFilename.c_str(), m_heightMapFractal);
		}
	}
	for (const SystemBody *kid : m_children) {
		assert(kid->m_parent == this);
		char buf[32];
		snprintf(buf, sizeof(buf), "%s\t", indent);
		kid->Dump(file, buf);
	}
	fprintf(file, "%s}\n", indent);
}

void SystemBody::ClearParentAndChildPointers()
{
	PROFILE_SCOPED()
	for (std::vector<SystemBody *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->ClearParentAndChildPointers();
	m_parent = 0;
	m_children.clear();
}

SystemBody *SystemBody::GetNearestJumpable()
{
	PROFILE_SCOPED()
	if (IsJumpable()) return this;
	// trying to find a jumpable parent
	SystemBody *result = this;
	while (result->GetParent()) { // we need to remember the last non-null pointer - this is the root systembody
		result = result->GetParent();
		if (result->IsJumpable()) return result;
	}

	//  Now we climbed to the very top of the hierarchy and did not find a jumpable
	//  parent - we will have to search purely geometrically
	//  we go through all the bodies of the system, determining the coordinates
	//  of jumpable objects, and also remember the coordinates of the current
	//  object
	//  the result variable now contains the root systembody
	std::vector<std::pair<SystemBody *, vector3d>> jumpables;
	vector3d this_position(0.0);
	std::function<void(SystemBody *, vector3d)> collect_positions = [&](SystemBody *s, vector3d pos) {
		if (s->IsJumpable() || s->HasChildren() || s == this) {
			// if the body has a zero orbit or it is a surface port - we assume that
			// it is at the same point with the parent, just don't touch pos
			if (!is_zero_general(s->GetOrbit().GetSemiMajorAxis()) && s->GetType() != SystemBody::TYPE_STARPORT_SURFACE)
				pos += s->GetOrbit().OrbitalPosAtTime(Pi::game->GetTime());
			if (s->IsJumpable())
				jumpables.emplace_back(s, pos);
			else if (s == this) // the current body is definitely not jumpable, otherwise we would not be here
				this_position = pos;
			if (s->HasChildren())
				for (auto kid : s->GetChildren())
					collect_positions(kid, pos);
		}
	};
	collect_positions(result, vector3d(0.0));
	// there can be no systems without jumpable systembodies!
	assert(jumpables.size());
	// looking for the closest jumpable to given systembody
	result = jumpables[0].first;
	double best_dist_sqr = (this_position - jumpables[0].second).LengthSqr();
	size_t i = 1;
	while (i < jumpables.size()) {
		double dist_sqr = (this_position - jumpables[i].second).LengthSqr();
		if (dist_sqr < best_dist_sqr) {
			best_dist_sqr = dist_sqr;
			result = jumpables[i].first;
		}
		++i;
	}
	return result;
}
