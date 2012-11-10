// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemGenerator.h"
#include "Sector.h"


//-----------------------------------------------------------------------------
// Build

const std::string SystemGenerator::Name()
{
	return m_sector.m_systems[m_path.systemIndex].name;
}

/*
 * 0 - ~500ly from sol: explored
 * ~500ly - ~700ly (65-90 sectors): gradual
 * ~700ly+: unexplored
 */
const bool SystemGenerator::Unexplored() 
{
	int dist = isqrt(1 + m_path.sectorX*m_path.sectorX + m_path.sectorY*m_path.sectorY + m_path.sectorZ*m_path.sectorZ);
	return (dist > 90) || (dist > 65 && rand1().Int32(dist) > 40);
}

//-----------------------------------------------------------------------------
// State

MTRand& SystemGenerator::rand1() 
{
	if (!m_rand1) {
		unsigned long _init[6] = { m_path.systemIndex, Uint32(m_path.sectorX), Uint32(m_path.sectorY), Uint32(m_path.sectorZ), UNIVERSE_SEED, Uint32(m_seed) };
		m_rand1 = new MTRand(_init, 6);
	}
	return *m_rand1;
}


//-----------------------------------------------------------------------------
// Construction/Destruction

SystemGenerator::SystemGenerator(SystemPath& path): m_path(path), m_rand1(0), m_sector(m_path.sectorX, m_path.sectorY, m_path.sectorZ) 
{
	assert(m_path.systemIndex >= 0 && m_path.systemIndex < s.m_systems.size());
	m_seed = m_sector.m_systems[m_path.systemIndex].seed;
}

SystemGenerator::~SystemGenerator() 
{
	if (m_rand1) delete m_rand1;
}
