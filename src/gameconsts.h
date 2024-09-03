// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAMECONSTS_H
#define _GAMECONSTS_H

#include <cstdint>

static const double PHYSICS_HZ = 60.0;

static const double MAX_LANDING_SPEED = 30.0; // m/sec
static const double LIGHT_SPEED = 3e8; // m/sec

static const uint32_t UNIVERSE_SEED = 0xabcd1234;

static const double EARTH_RADIUS = 6378135.0; // m
static const double EARTH_MASS = 5.9742e24; // Kg
static const double SOL_RADIUS = 6.955e8; // m
static const double SOL_MASS = 1.98892e30; // Kg

static const double AU = 149598000000.0; // m
static const double G = 6.67428e-11;

static const double EARTH_ATMOSPHERE_SURFACE_DENSITY = 1.225;
static const double GAS_CONSTANT_R = 8.3144621;

const double PA_2_ATMOS = 1.0 / 101325.0; // pascal -> atm

#endif /* _GAMECONSTS_H */
