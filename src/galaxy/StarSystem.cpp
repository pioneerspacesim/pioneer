// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystem.h"
#include "Sector.h"
#include "Galaxy.h"
#include "GalaxyCache.h"
#include "GalaxyGenerator.h"
#include "Factions.h"

#include "Serializer.h"
#include "Pi.h"
#include "LuaEvent.h"
#include "enum_table.h"
#include <map>
#include <string>
#include <algorithm>
#include "utils.h"
#include "Orbit.h"
#include "Lang.h"
#include "StringF.h"
#include <SDL_stdinc.h>
#include "EnumStrings.h"

//#define DEBUG_DUMP

// indexed by enum type turd
const Uint8 StarSystem::starColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 128, 0, 0 }, // brown dwarf
	{ 102, 102, 204 }, // white dwarf
	{ 255, 51, 0 }, // M
	{ 255, 153, 26 }, // K
	{ 255, 255, 102 }, // G
	{ 255, 255, 204 }, // F
	{ 255, 255, 255 }, // A
	{ 178, 178, 255 }, // B
	{ 255, 178, 255 }, // O
	{ 255, 51, 0 }, // M Giant
	{ 255, 153, 26 }, // K Giant
	{ 255, 255, 102 }, // G Giant
	{ 255, 255, 204 }, // F Giant
	{ 255, 255, 255 }, // A Giant
	{ 178, 178, 255 }, // B Giant
	{ 255, 178, 255 }, // O Giant
	{ 255, 51, 0 }, // M Super Giant
	{ 255, 153, 26 }, // K Super Giant
	{ 255, 255, 102 }, // G Super Giant
	{ 255, 255, 204 }, // F Super Giant
	{ 255, 255, 255 }, // A Super Giant
	{ 178, 178, 255 }, // B Super Giant
	{ 255, 178, 255 }, // O Super Giant
	{ 255, 51, 0 }, // M Hyper Giant
	{ 255, 153, 26 }, // K Hyper Giant
	{ 255, 255, 102 }, // G Hyper Giant
	{ 255, 255, 204 }, // F Hyper Giant
	{ 255, 255, 255 }, // A Hyper Giant
	{ 178, 178, 255 }, // B Hyper Giant
	{ 255, 178, 255 }, // O Hyper Giant
	{ 255, 51, 0 }, // Red/M Wolf Rayet Star
	{ 178, 178, 255 }, // Blue/B Wolf Rayet Star
	{ 255, 178, 255 }, // Purple-Blue/O Wolf Rayet Star
	{ 76, 178, 76 }, // Stellar Blackhole
	{ 51, 230, 51 }, // Intermediate mass Black-hole
	{ 0, 255, 0 }, // Super massive black hole
};

// indexed by enum type turd
const Uint8 StarSystem::starRealColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 128, 0, 0 }, // brown dwarf
	{ 255, 255, 255 }, // white dwarf
	{ 255, 128, 51 }, // M
	{ 255, 255, 102 }, // K
	{ 255, 255, 242 }, // G
	{ 255, 255, 255 }, // F
	{ 255, 255, 255 }, // A
	{ 204, 204, 255 }, // B
	{ 255, 204, 255 },  // O
	{ 255, 128, 51 }, // M Giant
	{ 255, 255, 102 }, // K Giant
	{ 255, 255, 242 }, // G Giant
	{ 255, 255, 255 }, // F Giant
	{ 255, 255, 255 }, // A Giant
	{ 204, 204, 255 }, // B Giant
	{ 255, 204, 255 },  // O Giant
	{ 255, 128, 51 }, // M Super Giant
	{ 255, 255, 102 }, // K Super Giant
	{ 255, 255, 242 }, // G Super Giant
	{ 255, 255, 255 }, // F Super Giant
	{ 255, 255, 255 }, // A Super Giant
	{ 204, 204, 255 }, // B Super Giant
	{ 255, 204, 255 },  // O Super Giant
	{ 255, 128, 51 }, // M Hyper Giant
	{ 255, 255, 102 }, // K Hyper Giant
	{ 255, 255, 242 }, // G Hyper Giant
	{ 255, 255, 255 }, // F Hyper Giant
	{ 255, 255, 255 }, // A Hyper Giant
	{ 204, 204, 255 }, // B Hyper Giant
	{ 255, 204, 255 },  // O Hyper Giant
	{ 255, 153, 153 }, // M WF
	{ 204, 204, 255 }, // B WF
	{ 255, 204, 255 },  // O WF
	{ 255, 255, 255 },  // small Black hole
	{ 16, 0, 20 }, // med BH
	{ 10, 0, 16 }, // massive BH
};

const double StarSystem::starLuminosities[] = {
	0,
	0.0003, // brown dwarf
	0.1, // white dwarf
	0.08, // M0
	0.38, // K0
	1.2, // G0
	5.1, // F0
	24.0, // A0
	100.0, // B0
	200.0, // O5
	1000.0, // M0 Giant
	2000.0, // K0 Giant
	4000.0, // G0 Giant
	6000.0, // F0 Giant
	8000.0, // A0 Giant
	9000.0, // B0 Giant
	12000.0, // O5 Giant
	12000.0, // M0 Super Giant
	14000.0, // K0 Super Giant
	18000.0, // G0 Super Giant
	24000.0, // F0 Super Giant
	30000.0, // A0 Super Giant
	50000.0, // B0 Super Giant
	100000.0, // O5 Super Giant
	125000.0, // M0 Hyper Giant
	150000.0, // K0 Hyper Giant
	175000.0, // G0 Hyper Giant
	200000.0, // F0 Hyper Giant
	200000.0, // A0 Hyper Giant
	200000.0, // B0 Hyper Giant
	200000.0, // O5 Hyper Giant
	50000.0, // M WF
	100000.0, // B WF
	200000.0, // O WF
	0.0003, // Stellar Black hole
	0.00003, // IM Black hole
	0.000003, // Supermassive Black hole
};

const float StarSystem::starScale[] = {  // Used in sector view
	0,
	0.6f, // brown dwarf
	0.5f, // white dwarf
	0.7f, // M
	0.8f, // K
	0.8f, // G
	0.9f, // F
	1.0f, // A
	1.1f, // B
	1.1f, // O
	1.3f, // M Giant
	1.2f, // K G
	1.2f, // G G
	1.2f, // F G
	1.1f, // A G
	1.1f, // B G
	1.2f, // O G
	1.8f, // M Super Giant
	1.6f, // K SG
	1.5f, // G SG
	1.5f, // F SG
	1.4f, // A SG
	1.3f, // B SG
	1.3f, // O SG
	2.5f, // M Hyper Giant
	2.2f, // K HG
	2.2f, // G HG
	2.1f, // F HG
	2.1f, // A HG
	2.0f, // B HG
	1.9f, // O HG
	1.1f, // M WF
	1.3f, // B WF
	1.6f, // O WF
	1.0f, // Black hole
	2.5f, // Intermediate-mass blackhole
	4.0f  // Supermassive blackhole
};

SystemBody::BodySuperType SystemBody::GetSuperType() const
{
	PROFILE_SCOPED()
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
		if (m_mass > 80) return Lang::MEDIUM_GAS_GIANT;
		else return Lang::SMALL_GAS_GIANT;
	case TYPE_PLANET_ASTEROID: return Lang::ASTEROID;
	case TYPE_PLANET_TERRESTRIAL: {
		std::string s;
		if (m_mass > fixed(2,1)) s = Lang::MASSIVE;
		else if (m_mass > fixed(3,2)) s = Lang::LARGE;
		else if (m_mass < fixed(1,10)) s = Lang::TINY;
		else if (m_mass < fixed(1,5)) s = Lang::SMALL;

		if (m_volcanicity > fixed(7,10)) {
			if (s.size()) s += Lang::COMMA_HIGHLY_VOLCANIC;
			else s = Lang::HIGHLY_VOLCANIC;
		}

		if (m_volatileIces + m_volatileLiquid > fixed(4,5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (m_averageTemp < fixed(250)) {
					s += Lang::ICE_WORLD;
				} else s += Lang::ROCKY_PLANET;
			} else {
				if (m_averageTemp < fixed(250)) {
					s += Lang::ICE_WORLD;
				} else {
					s += Lang::OCEANICWORLD;
				}
			}
		} else if (m_volatileLiquid > fixed(2,5)){
			if (m_averageTemp > fixed(250)) {
				s += Lang::PLANET_CONTAINING_LIQUID_WATER;
			} else {
				s += Lang::PLANET_WITH_SOME_ICE;
			}
		} else if (m_volatileLiquid > fixed(1,5)){
			s += Lang::ROCKY_PLANET_CONTAINING_COME_LIQUIDS;
		} else {
			s += Lang::ROCKY_PLANET;
		}

		if (m_volatileGas < fixed(1,100)) {
			s += Lang::WITH_NO_SIGNIFICANT_ATMOSPHERE;
		} else {
			std::string thickness;
			if (m_volatileGas < fixed(1,10)) thickness = Lang::TENUOUS;
			else if (m_volatileGas < fixed(1,5)) thickness = Lang::THIN;
			else if (m_volatileGas < fixed(2,1)) {}
			else if (m_volatileGas < fixed(4,1)) thickness = Lang::THICK;
			else thickness = Lang::VERY_DENSE;

			if (m_atmosOxidizing > fixed(95,100)) {
				s += Lang::WITH_A+thickness+Lang::O2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(7,10)) {
				s += Lang::WITH_A+thickness+Lang::CO2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(65,100)) {
				s += Lang::WITH_A+thickness+Lang::CO_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(55,100)) {
				s += Lang::WITH_A+thickness+Lang::CH4_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(3,10)) {
				s += Lang::WITH_A+thickness+Lang::H_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(2,10)) {
				s += Lang::WITH_A+thickness+Lang::HE_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(15,100)) {
				s += Lang::WITH_A+thickness+Lang::AR_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(1,10)) {
				s += Lang::WITH_A+thickness+Lang::S_ATMOSPHERE;
			} else {
				s += Lang::WITH_A+thickness+Lang::N_ATMOSPHERE;
			}
		}

		if (m_life > fixed(1,2)) {
			s += Lang::AND_HIGHLY_COMPLEX_ECOSYSTEM;
		} else if (m_life > fixed(1,10)) {
			s += Lang::AND_INDIGENOUS_PLANT_LIFE;
		} else if (m_life > fixed()) {
			s += Lang::AND_INDIGENOUS_MICROBIAL_LIFE;
		} else {
			s += ".";
		}

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
	case TYPE_STAR_O_SUPER_GIANT: return "icons/object_star_b_super_giant.png";// uses B type graphic for now
	case TYPE_STAR_M_HYPER_GIANT: return "icons/object_star_m_hyper_giant.png";
	case TYPE_STAR_K_HYPER_GIANT: return "icons/object_star_k_hyper_giant.png";
	case TYPE_STAR_G_HYPER_GIANT: return "icons/object_star_g_hyper_giant.png";
	case TYPE_STAR_F_HYPER_GIANT: return "icons/object_star_f_hyper_giant.png";
	case TYPE_STAR_A_HYPER_GIANT: return "icons/object_star_a_hyper_giant.png";
	case TYPE_STAR_B_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png";
	case TYPE_STAR_O_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png";// uses B type graphic for now
	case TYPE_STAR_M_WF: return "icons/object_star_m_wf.png";
	case TYPE_STAR_B_WF: return "icons/object_star_b_wf.png";
	case TYPE_STAR_O_WF: return "icons/object_star_o_wf.png";
	case TYPE_STAR_S_BH: return "icons/object_star_bh.png";
	case TYPE_STAR_IM_BH: return "icons/object_star_smbh.png";
	case TYPE_STAR_SM_BH: return "icons/object_star_smbh.png";
	case TYPE_PLANET_GAS_GIANT:
		if (m_mass > 800) {
			if (m_averageTemp > 1000) return "icons/object_planet_large_gas_giant_hot.png";
			else return "icons/object_planet_large_gas_giant.png";
		}
		if (m_mass > 300) {
			if (m_averageTemp > 1000) return "icons/object_planet_large_gas_giant_hot.png";
			else return "icons/object_planet_large_gas_giant.png";
		}
		if (m_mass > 80) {
			if (m_averageTemp > 1000) return "icons/object_planet_medium_gas_giant_hot.png";
			else return "icons/object_planet_medium_gas_giant.png";
		}
		else {
			if (m_averageTemp > 1000) return "icons/object_planet_small_gas_giant_hot.png";
			else return "icons/object_planet_small_gas_giant.png";
		}
	case TYPE_PLANET_ASTEROID:
		return "icons/object_planet_asteroid.png";
	case TYPE_PLANET_TERRESTRIAL:
		if (m_volatileLiquid > fixed(7,10)) {
			if (m_averageTemp > 250) return "icons/object_planet_water.png";
			else return "icons/object_planet_ice.png";
		}
		if ((m_life > fixed(9,10)) &&
			(m_volatileGas > fixed(6,10))) return "icons/object_planet_life.png";
		if ((m_life > fixed(8,10)) &&
			(m_volatileGas > fixed(5,10))) return "icons/object_planet_life6.png";
		if ((m_life > fixed(7,10)) &&
			(m_volatileGas > fixed(45,100))) return "icons/object_planet_life7.png";
		if ((m_life > fixed(6,10)) &&
			(m_volatileGas > fixed(4,10))) return "icons/object_planet_life8.png";
		if ((m_life > fixed(5,10)) &&
			(m_volatileGas > fixed(3,10))) return "icons/object_planet_life4.png";
		if ((m_life > fixed(4,10)) &&
			(m_volatileGas > fixed(2,10))) return "icons/object_planet_life5.png";
		if ((m_life > fixed(1,10)) &&
			(m_volatileGas > fixed(2,10))) return "icons/object_planet_life2.png";
		if (m_life > fixed(1,10)) return "icons/object_planet_life3.png";
		if (m_mass < fixed(1,100)) return "icons/object_planet_dwarf.png";
		if (m_mass < fixed(1,10)) return "icons/object_planet_small.png";
		if ((m_volatileLiquid < fixed(1,10)) &&
			(m_volatileGas > fixed(1,5))) return "icons/object_planet_desert.png";

		if (m_volatileIces + m_volatileLiquid > fixed(3,5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (m_averageTemp < 250)	return "icons/object_planet_ice.png";
			} else {
				if (m_averageTemp > 250) {
					return "icons/object_planet_water.png";
				} else return "icons/object_planet_ice.png";
			}
		}

		if (m_volatileGas > fixed(1,2)) {
			if (m_atmosOxidizing < fixed(1,2)) {
				if (m_averageTemp > 300) return "icons/object_planet_methane3.png";
				else if (m_averageTemp > 250) return "icons/object_planet_methane2.png";
				else return "icons/object_planet_methane.png";
			} else {
				if (m_averageTemp > 300) return "icons/object_planet_co2_2.png";
				else if (m_averageTemp > 250) {
					if ((m_volatileLiquid > fixed(3,10)) && (m_volatileGas > fixed(2,10)))
						return "icons/object_planet_co2_4.png";
					else return "icons/object_planet_co2_3.png";
				} else return "icons/object_planet_co2.png";
			}
		}

		if ((m_volatileLiquid > fixed(1,10)) &&
		   (m_volatileGas < fixed(1,10))) return "icons/object_planet_ice.png";
		if (m_volcanicity > fixed(7,10)) return "icons/object_planet_volcanic.png";
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

double SystemBody::GetMaxChildOrbitalDistance() const
{
	PROFILE_SCOPED()
	double max = 0;
	for (unsigned int i=0; i<m_children.size(); i++) {
		if (m_children[i]->m_orbMax.ToDouble() > max) {
			max = m_children[i]->m_orbMax.ToDouble();
		}
	}
	return AU * max;
}

bool SystemBody::IsCoOrbitalWith(const SystemBody* other) const
{
	if(m_parent && m_parent->GetType()==SystemBody::TYPE_GRAVPOINT
	&& ((m_parent->m_children[0] == this && m_parent->m_children[1] == other)
	|| (m_parent->m_children[1] == this && m_parent->m_children[0] == other)))
		return true;
	return false;
}

bool SystemBody::IsCoOrbital() const
{
	if(m_parent	&& m_parent->GetType()==SystemBody::TYPE_GRAVPOINT && (m_parent->m_children[0] == this || m_parent->m_children[1] == this))
		return true;
	return false;
}

double SystemBody::CalcSurfaceGravity() const
{
	PROFILE_SCOPED()
	double r = GetRadius();
	if (r > 0.0) {
		return G * GetMass() / pow(r, 2);
	} else {
		return 0.0;
	}
}

SystemBody *StarSystem::GetBodyByPath(const SystemPath &path) const
{
	PROFILE_SCOPED()
	assert(m_path.IsSameSystem(path));
	assert(path.IsBodyPath());
	assert(path.bodyIndex < m_bodies.size());

	return m_bodies[path.bodyIndex].Get();
}

SystemPath StarSystem::GetPathOf(const SystemBody *sbody) const
{
	return sbody->GetPath();
}

SystemBody::SystemBody(const SystemPath& path, StarSystem *system) : m_parent(nullptr), m_path(path), m_seed(0), m_aspectRatio(1,1), m_orbMin(0),
	m_orbMax(0), m_rotationalPhaseAtStart(0), m_semiMajorAxis(0), m_eccentricity(0), m_orbitalOffset(0), m_axialTilt(0),
	m_inclination(0), m_averageTemp(0), m_type(TYPE_GRAVPOINT), m_isCustomBody(false), m_heightMapFractal(0), m_atmosDensity(0.0), m_system(system)
{
}

bool SystemBody::HasAtmosphere() const
{
	PROFILE_SCOPED()
	return (m_volatileGas > fixed(1,100));
}

bool SystemBody::IsScoopable() const
{
	PROFILE_SCOPED()
	return (GetSuperType() == SUPERTYPE_GAS_GIANT);
}

// Calculate parameters used in the atmospheric model for shaders
SystemBody::AtmosphereParameters SystemBody::CalcAtmosphereParams() const
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
	const double radiusPlanet_in_m = (m_radius.ToDouble()*EARTH_RADIUS);
	const double massPlanet_in_kg = (m_mass.ToDouble()*EARTH_MASS);
	const double g = G*massPlanet_in_kg/(radiusPlanet_in_m*radiusPlanet_in_m);

	double T = static_cast<double>(m_averageTemp);

	// XXX hack to avoid issues with sysgen giving 0 temps
	// temporary as part of sysgen needs to be rewritten before the proper fix can be used
	if (T < 1)
		T = 165;

	// We have two kinds of atmosphere: Earth-like and gas giant (hydrogen/helium)
	const double M = m_type == TYPE_PLANET_GAS_GIANT ? 0.0023139903 : 0.02897f; // in kg/mol

	float atmosScaleHeight = static_cast<float>(GAS_CONSTANT_R*T/(M*g));

	// min of 2.0 corresponds to a scale height of 1/20 of the planet's radius,
	params.atmosInvScaleHeight = std::max(20.0f, static_cast<float>(GetRadius() / atmosScaleHeight));
	// integrate atmospheric density between surface and this radius. this is 10x the scale
	// height, which should be a height at which the atmospheric density is negligible
	params.atmosRadius = 1.0f + static_cast<float>(10.0f * atmosScaleHeight) / GetRadius();

	params.planetRadius = static_cast<float>(radiusPlanet_in_m);

	return params;
}

/*
 * As my excellent comrades have pointed out, choices that depend on floating
 * point crap will result in different universes on different platforms.
 *
 * We must be sneaky and avoid floating point in these places.
 */
StarSystem::StarSystem(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache* cache, Random& rand)
	: m_galaxy(galaxy), m_path(path.SystemOnly()), m_numStars(0), m_isCustom(false),
	  m_faction(nullptr), m_explored(eEXPLORED_AT_START), m_exploredTime(0.0), m_econType(GalacticEconomy::ECON_MINING), m_seed(0),
	  m_commodityLegal(unsigned(GalacticEconomy::Commodity::COMMODITY_COUNT), true), m_cache(cache)
{
	PROFILE_SCOPED()
	memset(m_tradeLevel, 0, sizeof(m_tradeLevel));
}

StarSystem::GeneratorAPI::GeneratorAPI(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache* cache, Random& rand)
	: StarSystem(path, galaxy, cache, rand) { }

#ifdef DEBUG_DUMP
struct thing_t {
	SystemBody* obj;
	vector3d pos;
	vector3d vel;
};
void StarSystem::Dump()
{
	std::vector<SystemBody*> obj_stack;
	std::vector<vector3d> pos_stack;
	std::vector<thing_t> output;

	SystemBody *obj = m_rootBody;
	vector3d pos = vector3d(0.0);

	while (obj) {
		vector3d p2 = pos;
		if (obj->m_parent) {
			p2 = pos + obj->m_orbit.OrbitalPosAtTime(1.0);
			pos = pos + obj->m_orbit.OrbitalPosAtTime(0.0);
		}

		if ((obj->GetType() != SystemBody::TYPE_GRAVPOINT) &&
		    (obj->GetSuperType() != SystemBody::SUPERTYPE_STARPORT)) {
			struct thing_t t;
			t.obj = obj;
			t.pos = pos;
			t.vel = (p2-pos);
			output.push_back(t);
		}
		for (std::vector<SystemBody*>::iterator i = obj->m_children.begin();
				i != obj->m_children.end(); ++i) {
			obj_stack.push_back(*i);
			pos_stack.push_back(pos);
		}
		if (obj_stack.size() == 0) break;
		pos = pos_stack.back();
		obj = obj_stack.back();
		pos_stack.pop_back();
		obj_stack.pop_back();
	}

	FILE *f = fopen("starsystem.dump", "w");
	fprintf(f, "%lu bodies\n", output.size());
	fprintf(f, "0 steps\n");
	for (std::vector<thing_t>::iterator i = output.begin();
			i != output.end(); ++i) {
		fprintf(f, "B:%lf,%lf:%lf,%lf,%lf,%lf:%lf:%d:%lf,%lf,%lf\n",
				(*i).pos.x, (*i).pos.y, (*i).pos.z,
				(*i).vel.x, (*i).vel.y, (*i).vel.z,
				(*i).obj->GetMass(), 0,
				1.0, 1.0, 1.0);
	}
	fclose(f);
	Output("Junk dumped to starsystem.dump\n");
}
#endif /* DEBUG_DUMP */

void StarSystem::MakeShortDescription()
{
	PROFILE_SCOPED()
	if (GetExplored() == StarSystem::eUNEXPLORED)
		SetShortDesc(Lang::UNEXPLORED_SYSTEM_NO_DATA);

	else if (GetExplored() == StarSystem::eEXPLORED_BY_PLAYER)
		SetShortDesc(stringf(Lang::RECENTLY_EXPLORED_SYSTEM, formatarg("date", format_date_only(GetExploredTime()))));

	/* Total population is in billions */
	else if(GetTotalPop() == 0) {
		SetShortDesc(Lang::SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS);
	} else if (GetTotalPop() < fixed(1,10)) {
		switch (GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::SMALL_INDUSTRIAL_OUTPOST); break;
			case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::SOME_ESTABLISHED_MINING); break;
			case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::YOUNG_FARMING_COLONY); break;
		}
	} else if (GetTotalPop() < fixed(1,2)) {
		switch (GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::INDUSTRIAL_COLONY); break;
			case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::MINING_COLONY); break;
			case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::OUTDOOR_AGRICULTURAL_WORLD); break;
		}
	} else if (GetTotalPop() < fixed(5,1)) {
		switch (GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::HEAVY_INDUSTRY); break;
			case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::EXTENSIVE_MINING); break;
			case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::THRIVING_OUTDOOR_WORLD); break;
		}
	} else {
		switch (GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::INDUSTRIAL_HUB_SYSTEM); break;
			case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::VAST_STRIP_MINE); break;
			case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::HIGH_POPULATION_OUTDOOR_WORLD); break;
		}
	}
}


void StarSystem::ExploreSystem(double time)
{
	if (m_explored != eUNEXPLORED)
		return;
	m_explored = eEXPLORED_BY_PLAYER;
	m_exploredTime = time;
	RefCountedPtr<Sector> sec = m_galaxy->GetMutableSector(m_path);
	Sector::System& secsys = sec->m_systems[m_path.systemIndex];
	secsys.SetExplored(m_explored, m_exploredTime);
	MakeShortDescription();
	LuaEvent::Queue("onSystemExplored", this);
}

void SystemBody::Dump(FILE* file, const char* indent) const
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
	for (const SystemBody* kid : m_children) {
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
	for (std::vector<SystemBody*>::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->ClearParentAndChildPointers();
	m_parent = 0;
	m_children.clear();
}

StarSystem::~StarSystem()
{
	PROFILE_SCOPED()
	// clear parent and children pointers. someone (Lua) might still have a
	// reference to things that are about to be deleted
	m_rootBody->ClearParentAndChildPointers();
	if (m_cache)
		m_cache->RemoveFromAttic(m_path);
}

void StarSystem::Serialize(Serializer::Writer &wr, StarSystem *s)
{
	if (s) {
		wr.Byte(1);
		wr.Int32(s->m_path.sectorX);
		wr.Int32(s->m_path.sectorY);
		wr.Int32(s->m_path.sectorZ);
		wr.Int32(s->m_path.systemIndex);
	} else {
		wr.Byte(0);
	}
}

RefCountedPtr<StarSystem> StarSystem::Unserialize(RefCountedPtr<Galaxy> galaxy, Serializer::Reader &rd)
{
	if (rd.Byte()) {
		int sec_x = rd.Int32();
		int sec_y = rd.Int32();
		int sec_z = rd.Int32();
		int sys_idx = rd.Int32();
		return galaxy->GetStarSystem(SystemPath(sec_x, sec_y, sec_z, sys_idx));
	} else {
		return RefCountedPtr<StarSystem>(0);
	}
}

std::string StarSystem::ExportBodyToLua(FILE *f, SystemBody *body) {
	const int multiplier = 10000;
	int i;

	std::string code_name = body->GetName();
	std::transform(code_name.begin(), code_name.end(), code_name.begin(), ::tolower);
	code_name.erase(remove_if(code_name.begin(), code_name.end(), isspace), code_name.end());
	for(unsigned int j = 0; j < code_name.length(); j++) {
		if(code_name[j] == ',')
			code_name[j] = 'X';
		if(!((code_name[j] >= 'a' && code_name[j] <= 'z') ||
				(code_name[j] >= 'A' && code_name[j] <= 'Z') ||
				(code_name[j] >= '0' && code_name[j] <= '9')))
			code_name[j] = 'Y';
	}

	std::string code_list = code_name;

	for(i = 0; ENUM_BodyType[i].name != 0; i++) {
		if(ENUM_BodyType[i].value == body->GetType())
			break;
	}

	if(body->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
		fprintf(f,
			"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
				"\t:latitude(math.deg2rad(%.1f))\n"
                "\t:longitude(math.deg2rad(%.1f))\n",

				code_name.c_str(),
				body->GetName().c_str(), ENUM_BodyType[i].name,
				body->m_inclination.ToDouble()*180/M_PI,
				body->m_orbitalOffset.ToDouble()*180/M_PI
				);
	} else {

		fprintf(f,
				"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
				"\t:radius(f(%d,%d))\n"
				"\t:mass(f(%d,%d))\n",
				code_name.c_str(),
				body->GetName().c_str(), ENUM_BodyType[i].name,
				int(round(body->GetRadiusAsFixed().ToDouble()*multiplier)), multiplier,
				int(round(body->GetMassAsFixed().ToDouble()*multiplier)), multiplier
		);

		if(body->GetType() != SystemBody::TYPE_GRAVPOINT)
		fprintf(f,
				"\t:seed(%u)\n"
				"\t:temp(%d)\n"
				"\t:semi_major_axis(f(%d,%d))\n"
				"\t:eccentricity(f(%d,%d))\n"
				"\t:rotation_period(f(%d,%d))\n"
				"\t:axial_tilt(fixed.deg2rad(f(%d,%d)))\n"
				"\t:rotational_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
				"\t:orbital_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
				"\t:orbital_offset(fixed.deg2rad(f(%d,%d)))\n",
			body->GetSeed(), body->GetAverageTemp(),
			int(round(body->GetOrbit().GetSemiMajorAxis()/AU*multiplier)), multiplier,
			int(round(body->GetOrbit().GetEccentricity()*multiplier)), multiplier,
			int(round(body->m_rotationPeriod.ToDouble()*multiplier)), multiplier,
			int(round(body->GetAxialTilt()*multiplier)), multiplier,
			int(round(body->m_rotationalPhaseAtStart.ToDouble()*multiplier*180/M_PI)), multiplier,
			int(round(body->m_orbitalPhaseAtStart.ToDouble()*multiplier*180/M_PI)), multiplier,
			int(round(body->m_orbitalOffset.ToDouble()*multiplier*180/M_PI)), multiplier
		);

		if(body->GetType() == SystemBody::TYPE_PLANET_TERRESTRIAL)
			fprintf(f,
					"\t:metallicity(f(%d,%d))\n"
					"\t:volcanicity(f(%d,%d))\n"
					"\t:atmos_density(f(%d,%d))\n"
					"\t:atmos_oxidizing(f(%d,%d))\n"
					"\t:ocean_cover(f(%d,%d))\n"
					"\t:ice_cover(f(%d,%d))\n"
					"\t:life(f(%d,%d))\n",
				int(round(body->GetMetallicity()*multiplier)), multiplier,
				int(round(body->GetVolcanicity()*multiplier)), multiplier,
				int(round(body->GetVolatileGas()*multiplier)), multiplier,
				int(round(body->GetAtmosOxidizing()*multiplier)), multiplier,
				int(round(body->GetVolatileLiquid()*multiplier)), multiplier,
				int(round(body->GetVolatileIces()*multiplier)), multiplier,
				int(round(body->GetLife()*multiplier)), multiplier
			);
	}

	fprintf(f, "\n");

	if(body->m_children.size() > 0) {
		code_list = code_list + ", \n\t{\n";
		for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
			code_list = code_list + "\t" + ExportBodyToLua(f, body->m_children[ii]) + ", \n";
		}
		code_list = code_list + "\t}";
	}

	return code_list;

}

std::string StarSystem::GetStarTypes(SystemBody *body) {
	int i = 0;
	std::string types = "";

	if(body->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		for(i = 0; ENUM_BodyType[i].name != 0; i++) {
			if(ENUM_BodyType[i].value == body->GetType())
				break;
		}

		types = types + "'" + ENUM_BodyType[i].name + "', ";
	}

	for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
		types = types + GetStarTypes(body->m_children[ii]);
	}

	return types;
}

void StarSystem::ExportToLua(const char *filename) {
	FILE *f = fopen(filename,"w");
	int j;

	if(f == 0)
		return;

	fprintf(f,"-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details\n");
	fprintf(f,"-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt\n\n");

	std::string stars_in_system = GetStarTypes(m_rootBody.Get());

	for(j = 0; ENUM_PolitGovType[j].name != 0; j++) {
		if(ENUM_PolitGovType[j].value == GetSysPolit().govType)
			break;
	}

	fprintf(f,"local system = CustomSystem:new('%s', { %s })\n\t:govtype('%s')\n\t:short_desc('%s')\n\t:long_desc([[%s]])\n\n",
			GetName().c_str(), stars_in_system.c_str(), ENUM_PolitGovType[j].name, GetShortDescription().c_str(), GetLongDescription().c_str());

	fprintf(f, "system:bodies(%s)\n\n", ExportBodyToLua(f, m_rootBody.Get()).c_str());

	RefCountedPtr<const Sector> sec = m_galaxy->GetSector(GetPath());
	SystemPath pa = GetPath();

	fprintf(f, "system:add_to_sector(%d,%d,%d,v(%.4f,%.4f,%.4f))\n",
			pa.sectorX, pa.sectorY, pa.sectorZ,
			sec->m_systems[pa.systemIndex].GetPosition().x/Sector::SIZE,
			sec->m_systems[pa.systemIndex].GetPosition().y/Sector::SIZE,
			sec->m_systems[pa.systemIndex].GetPosition().z/Sector::SIZE);

	fclose(f);
}

void StarSystem::Dump(FILE* file, const char* indent, bool suppressSectorData) const
{
	if (suppressSectorData) {
		fprintf(file, "%sStarSystem {%s\n", indent, m_hasCustomBodies ? " CUSTOM-ONLY" : m_isCustom ? " CUSTOM" : "");
	} else {
		fprintf(file, "%sStarSystem(%d,%d,%d,%u) {\n", indent, m_path.sectorX, m_path.sectorY, m_path.sectorZ, m_path.systemIndex);
		fprintf(file, "%s\t\"%s\"\n", indent, m_name.c_str());
		fprintf(file, "%s\t%sEXPLORED%s\n", indent, GetUnexplored() ? "UN" : "", m_hasCustomBodies ? ", CUSTOM-ONLY" : m_isCustom ? ", CUSTOM" : "");
		fprintf(file, "%s\tfaction %s%s%s\n", indent, m_faction ? "\"" : "NONE", m_faction ? m_faction->name.c_str() : "", m_faction ? "\"" : "");
		fprintf(file, "%s\tseed %u\n", indent, static_cast<Uint32>(m_seed));
		fprintf(file, "%s\t%u stars%s\n", indent, m_numStars, m_numStars > 0 ? " {" : "");
		assert(m_numStars == m_stars.size());
		for (unsigned i = 0; i < m_numStars; ++i)
			fprintf(file, "%s\t\t%s\n", indent, EnumStrings::GetString("BodyType", m_stars[i]->GetType()));
		if (m_numStars > 0) fprintf(file, "%s\t}\n", indent);
	}
	fprintf(file, "%s\t%zu bodies, %zu spaceports \n", indent, m_bodies.size(), m_spaceStations.size());
	fprintf(file, "%s\tpopulation %.0f\n", indent, m_totalPop.ToDouble() * 1e9);
	fprintf(file, "%s\tgovernment %s/%s, lawlessness %.2f\n", indent, m_polit.GetGovernmentDesc(), m_polit.GetEconomicDesc(),
		m_polit.lawlessness.ToDouble() * 100.0);
	fprintf(file, "%s\teconomy type%s%s%s\n", indent, m_econType == 0 ? " NONE" : m_econType & GalacticEconomy::ECON_AGRICULTURE ? " AGRICULTURE" : "",
		m_econType & GalacticEconomy::ECON_INDUSTRY ? " INDUSTRY" : "", m_econType & GalacticEconomy::ECON_MINING ? " MINING" : "");
	fprintf(file, "%s\thumanProx %.2f\n", indent, m_humanProx.ToDouble() * 100.0);
	fprintf(file, "%s\tmetallicity %.2f, industrial %.2f, agricultural %.2f\n", indent, m_metallicity.ToDouble() * 100.0,
		m_industrial.ToDouble() * 100.0, m_agricultural.ToDouble() * 100.0);
	fprintf(file, "%s\ttrade levels {\n", indent);
	for (int i = 1; i < GalacticEconomy::COMMODITY_COUNT; ++i) {
		fprintf(file, "%s\t\t%s = %d\n", indent, EnumStrings::GetString("CommodityType", i), m_tradeLevel[i]);
	}
	fprintf(file, "%s\t}\n", indent);
	if (m_rootBody) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%s\t", indent);
		assert(m_rootBody->GetPath().IsSameSystem(m_path));
		m_rootBody->Dump(file, buf);
	}
	fprintf(file, "%s}\n", indent);
}
