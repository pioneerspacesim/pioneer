// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef MOUNT_H
#define MOUNT_H

#include <string>
#include <vector>
#include "vector3.h"

enum GunDir {
	GUN_FRONT,
	GUN_REAR,
	GUNMOUNT_MAX = 2
};

// Structure holding name, position and direction of a mount (loaded from Model data)
// TODO: Merge these data with data of Shiptype (as in FixedGuns)
struct Mount {
	Mount() = default;

	Mount(std::string name_, std::vector<vector3d> locs_, GunDir dir_) :
		name(name_),
		locs(locs_),
		dir(dir_)
	{}

	std::string name;
	std::vector<vector3d> locs;
	GunDir dir;
};

typedef std::vector<Mount> GunMounts;

#endif // MOUNT_H
