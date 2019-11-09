// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef CUSTOMSYSTEMDB_H
#define CUSTOMSYSTEMDB_H

#include "SystemPath.h"

class CustomSystem;
class Galaxy;

class CustomSystemsDatabase {
public:
	CustomSystemsDatabase(Galaxy *galaxy, const std::string &customSysDir) :
		m_galaxy(galaxy),
		m_customSysDirectory(customSysDir) {}
	~CustomSystemsDatabase();

	void Init();

	typedef std::vector<const CustomSystem *> SystemList;
	// XXX this is not as const-safe as it should be
	const SystemList &GetCustomSystemsForSector(int sectorX, int sectorY, int sectorZ) const;
	void AddCustomSystem(const SystemPath &path, CustomSystem *csys);
	Galaxy *GetGalaxy() const { return m_galaxy; }

private:
	typedef std::map<SystemPath, CustomSystemsDatabase::SystemList> SectorMap;

	Galaxy *const m_galaxy;
	const std::string m_customSysDirectory;
	SectorMap m_sectorMap;
	static const CustomSystemsDatabase::SystemList s_emptySystemList; // see: Null Object pattern
};

#endif // CUSTOMSYSTEMDB_H
