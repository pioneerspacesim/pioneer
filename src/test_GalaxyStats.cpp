// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <iostream>
#include "FileSystem.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "mtrand.h"

using namespace std;

// Brute force crude stats for galaxy. May take a while, depending on settings!
int main(int argc, char *argv[]) {

	const int galaxy_depth = 5000; // assume 5k light years deep

	// Edge sectors of the galaxy
	const int minx = -(Galaxy::GALAXY_RADIUS / Sector::SIZE) - (Galaxy::SOL_OFFSET_X / Sector::SIZE);
	const int maxx =  (Galaxy::GALAXY_RADIUS / Sector::SIZE) - (Galaxy::SOL_OFFSET_X / Sector::SIZE);
	const int miny = -(Galaxy::GALAXY_RADIUS / Sector::SIZE) - (Galaxy::SOL_OFFSET_Y / Sector::SIZE);
	const int maxy =  (Galaxy::GALAXY_RADIUS / Sector::SIZE) - (Galaxy::SOL_OFFSET_Y / Sector::SIZE);
	const int minz = -galaxy_depth / Sector::SIZE / 2;
	const int maxz =  galaxy_depth / Sector::SIZE / 2;

	// Number of sectors to skip on each axis between each sample.
	const int skip_sectors  = 25;

	// Total number of sectors in the galaxy
	const double galaxy_volume = double(maxx-minx) * double(maxy-miny) * double(maxz-minz);

	cout << "--------------------" << endl;
	cout << "Getting galaxy stats" << endl;
	cout << "--------------------" << endl;

	FileSystem::Init();
	Galaxy::Init();

	MTRand rng(0);

	Uint64 systemCount=0;
	Uint64 sectorCount=0;
	Uint64 starCount  =0;

	Uint64 starTypeCount[SystemBody::TYPE_STAR_MAX];
	for (int i=0; i <= SystemBody::TYPE_STAR_MAX; ++i) starTypeCount[i] = 0;

	for (int y=miny; y<maxy; y+=skip_sectors) {
		for (int x=minx; x<maxx; x+=skip_sectors) {
			for (int z=minz; z<maxz; z+=skip_sectors) {
				if (Galaxy::GetSectorDensity(x,y,z) > 0) {
					Sector sector(x, y, z);
					systemCount += sector.m_systems.size();
					for (std::vector<Sector::System>::iterator sys = sector.m_systems.begin(); sys !=sector.m_systems.end(); ++sys) {
						starCount += (*sys).numStars;
						for (int i=0; i < (*sys).numStars; ++i) starTypeCount[(*sys).starType[i]]++;
					}
				}
				sectorCount++; // accurate even if the loop structure is changed.
			}
		}
	}

	cout << "Sectors checked      = " << sectorCount << " ("  << double(sectorCount) / galaxy_volume * 100.0 << "%)" << endl;
	cout << "Systems per sector   = " << double(systemCount) / double(sectorCount) << endl;
	cout << "Stars per sector     = " << double(starCount)   / double(sectorCount) << endl;
	cout << "Estimated star count = " << double(starCount)   / (double(sectorCount) / galaxy_volume) / 1000000000.0 << " billion" << endl;

	for (int i=SystemBody::TYPE_STAR_MIN; i <= SystemBody::TYPE_STAR_MAX; ++i)
		cout << "Type " << i << " stars = " << starTypeCount[i] << " (" << double(starTypeCount[i]) / double(starCount) * 100.0 << "%)" << endl;

	return 0;
}
