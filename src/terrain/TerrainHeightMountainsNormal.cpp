#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainHeightFractal<TerrainHeightMountainsNormal>::GetHeightFractalName() const { return "MountainsNormal"; }

template <>
TerrainHeightFractal<TerrainHeightMountainsNormal>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 1e7), 10000*m_fracmult);
	SetFracDef(1, m_maxHeightInMeters*0.00000000001, 100.0, 10*m_fracmult);
	SetFracDef(2, m_maxHeightInMeters*0.0000001, m_rand.Double(500, 2e3), 1000*m_fracmult);
	SetFracDef(3, m_maxHeightInMeters*0.00002, m_rand.Double(1500, 1e4), 100*m_fracmult);
	SetFracDef(4, m_maxHeightInMeters*0.08, 1e4, 100*m_fracmult);
	SetFracDef(5, m_maxHeightInMeters*0.2, 1e5, 100*m_fracmult);
	SetFracDef(6, m_maxHeightInMeters*0.5, 1e6, 1000*m_fracmult);
	SetFracDef(7, m_maxHeightInMeters*0.5, m_rand.Double(1e6,1e7), 1000*m_fracmult);
	SetFracDef(8, m_maxHeightInMeters, m_rand.Double(3e6, 1e7), 1000*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightMountainsNormal>::GetHeight(const vector3d &p)
	//This is among the most complex of terrains, so I'll use this as an example:
{
	//We need a continental pattern to place our noise onto, the 0.7*ridged_octavnoise..... is important here
	// for making 'broken up' coast lines, as opposed to circular land masses, it will reduce the frequency of our
	// continents depending on the ridged noise value, we subtract sealevel so that sea level will have an effect on the continents size
	double continents = octavenoise(GetFracDef(0), 0.7*
		ridged_octavenoise(GetFracDef(8), 0.58, p), p) - m_sealevel*0.65;
	// if there are no continents on an area, we want it to be sea level
	if (continents < 0) return 0;
	double n = continents - (GetFracDef(0).amplitude*m_sealevel*0.5);
	// we save the height n now as a constant h
	const double h = n;
	//We don't want to apply noise to sea level n=0
	if (n > 0.0) {
		//large mountainous shapes
		n += h*0.2*ridged_octavenoise(GetFracDef(7), 
			0.5*octavenoise(GetFracDef(6), 0.5, p), p);

		// This smoothes edges near the coast, we cant have vertical terrain its not handled correctly.
		if (n < 0.4){
			n += n*1.25*ridged_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.3, 0.7)*
				ridged_octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += 0.5*ridged_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.3, 0.7)*
				ridged_octavenoise(GetFracDef(5), 0.5, p), p);
		}

		if (n < 0.2){
			n += n*15.0*river_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.5, 0.7), p);
		} else {
			n += 3.0*river_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.5, 0.7), p);
		}

		if (n < 0.4){
			n += n*billow_octavenoise(GetFracDef(6), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += (0.16/n)*billow_octavenoise(GetFracDef(6), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		}

		if (n < 0.2){
			n += n*billow_octavenoise(GetFracDef(5), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += (0.04/n)*billow_octavenoise(GetFracDef(5), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		}
		//smaller ridged mountains
		n += n*0.7*ridged_octavenoise(GetFracDef(5), 
			0.5*octavenoise(GetFracDef(6), 0.5, p), p);

		n = (n/2)+(n*n);
		
		//jagged surface for mountains
		//This is probably using far too much noise, some of it is just not needed
		// More specifically this: Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.5, p),
		//		 0.5*octavenoise(GetFracDef(3), 0.5, p), 
		//		 0.5*octavenoise(GetFracDef(3), 0.5, p))
		//should probably be: Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.5, p),
		//		 0.1, 
		//		 0.5)  But I have no time for testing
		if (n > 0.25) {
			n += (n-0.25)*0.1*octavenoise(GetFracDef(3), 
				Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.5, p),
				 0.5*octavenoise(GetFracDef(3), 0.5, p), 
				 0.5*octavenoise(GetFracDef(3), 0.5, p)), p); //[4]?
		} 
		
		if (n > 0.2 && n <= 0.25) {
			n += (0.25-n)*0.2*ridged_octavenoise(GetFracDef(3), 
				Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.5, p),
				 0.5*octavenoise(GetFracDef(3), 0.5, p), 
				 0.5*octavenoise(GetFracDef(4), 0.5, p)), p);
		} else if (n > 0.05) {
			n += ((n-0.05)/15)*ridged_octavenoise(GetFracDef(3), 
				Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.5, p),
				 0.5*octavenoise(GetFracDef(3), 0.5, p), 
				 0.5*octavenoise(GetFracDef(4), 0.5, p)), p);
		}
		n = n*0.2;

		if (n < 0.01){
			n += n*voronoiscam_octavenoise(GetFracDef(3), 
				Clamp(h*0.00002, 0.5, 0.5), p);
		} else if (n <0.02){
			n += 0.01*voronoiscam_octavenoise(GetFracDef(3), 
				Clamp(h*0.00002, 0.5, 0.5), p);
		} else {
			n += (0.02/n)*0.01*voronoiscam_octavenoise(GetFracDef(3), 
				Clamp(h*0.00002, 0.5, 0.5), p);
		}

		if (n < 0.001){
			n += n*3*dunes_octavenoise(GetFracDef(2), 
				1.0*octavenoise(GetFracDef(2), 0.5, p), p);
		} else if (n <0.01){
			n += 0.003*dunes_octavenoise(GetFracDef(2), 
				1.0*octavenoise(GetFracDef(2), 0.5, p), p);
		} else {
			n += (0.01/n)*0.003*dunes_octavenoise(GetFracDef(2), 
				1.0*octavenoise(GetFracDef(2), 0.5, p), p);
		}

		if (n < 0.001){
			n += n*0.2*ridged_octavenoise(GetFracDef(1), 
				0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		} else if (n <0.01){
			n += 0.0002*ridged_octavenoise(GetFracDef(1), 
				0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		} else {
			n += (0.01/n)*0.0002*ridged_octavenoise(GetFracDef(1), 
				0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		}

		if (n < 0.1){
			n += n*0.05*dunes_octavenoise(GetFracDef(2), 
				n*river_octavenoise(GetFracDef(2), 0.5, p), p);
		} else if (n <0.2){
			n += 0.005*dunes_octavenoise(GetFracDef(2), 
				((n*n*10.0)+(3*(n-0.1)))*
				river_octavenoise(GetFracDef(2), 0.5, p), p);
		} else {
			n += (0.2/n)*0.005*dunes_octavenoise(GetFracDef(2), 
				Clamp(0.7-(1-(5*n)), 0.0, 0.7)*
				river_octavenoise(GetFracDef(2), 0.5, p), p);
		}
 
		//terrain is too mountainous, so we reduce the height
		n *= 0.3;

	}
	
	n = m_maxHeight*n;
	return (n > 0.0 ? n : 0.0); 
}
