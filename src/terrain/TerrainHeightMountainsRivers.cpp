#include "Terrain.h"
#include "TerrainNoise.h"
#include "TerrainFeature.h"

using namespace TerrainNoise;
using namespace TerrainFeature;

template <>
const char *TerrainHeightFractal<TerrainHeightMountainsRivers>::GetHeightFractalName() const { return "MountainsRivers"; }

template <>
TerrainHeightFractal<TerrainHeightMountainsRivers>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	SetFracDef(0, m_maxHeightInMeters, m_rand.Double(1e6, 2e6), 10*m_fracmult);
	SetFracDef(1, m_maxHeightInMeters, 15e6, 100.0*m_fracmult);
	SetFracDef(2, m_maxHeightInMeters*0.0000001, m_rand.Double(500, 2e3), 10*m_fracmult);
	SetFracDef(3, m_maxHeightInMeters*0.00002, m_rand.Double(1500, 1e4), 10*m_fracmult);
	SetFracDef(4, m_maxHeightInMeters*0.08, 1e4, 10*m_fracmult);
	SetFracDef(5, m_maxHeightInMeters*0.2, 1e5, 10*m_fracmult);
	SetFracDef(6, m_maxHeightInMeters*0.5, 1e6, 100*m_fracmult);
	SetFracDef(7, m_maxHeightInMeters*0.5, m_rand.Double(1e6,5e6), 100*m_fracmult);
	SetFracDef(8, m_maxHeightInMeters, m_rand.Double(12e5, 22e5), 10*m_fracmult);
	SetFracDef(9, m_maxHeightInMeters, 1e7, 100.0*m_fracmult);
}

template <>
double TerrainHeightFractal<TerrainHeightMountainsRivers>::GetHeight(const vector3d &p)
{
	double continents = octavenoise(GetFracDef(0), 0.7*
		ridged_octavenoise(GetFracDef(8), 0.58, p), p) - m_sealevel*0.65;
	if (continents < 0) return 0;
	double n = (river_function(GetFracDef(9), p)*
		river_function(GetFracDef(7), p)*
		river_function(GetFracDef(6), p)*
		canyon3_normal_function(GetFracDef(1), p)*continents) -
		(GetFracDef(0).amplitude*m_sealevel*0.1);
	n *= 0.5;

	double h = n;

	if (n > 0.0) {
		// smooth in hills at shore edges
		//large mountainous shapes
		n += h*river_octavenoise(GetFracDef(7), 
			0.5*octavenoise(GetFracDef(6), 0.5, p), p);

		//if (n < 0.2) n += canyon3_billow_function(GetFracDef(9), p) * n * 5;
		//else if (n < 0.4) n += canyon3_billow_function(GetFracDef(9), p);
		//else n += canyon3_billow_function(GetFracDef(9), p) * (0.4/n);
		//n += -0.5;
	}

	if (n > 0.0) {
		if (n < 0.4){
			n += n*2.5*river_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.3, 0.7)*
				ridged_octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += 1.0*river_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.3, 0.7)*
				ridged_octavenoise(GetFracDef(5), 0.5, p), p);
		}
	}

	if (n > 0.0) {
		if (n < 0.2){
			n += n*5.0*billow_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.5, 0.7), p);
		} else {
			n += billow_octavenoise(GetFracDef(6), 
				Clamp(h*0.00002, 0.5, 0.7), p);
		}
	}

	if (n > 0.0) {
		if (n < 0.4){
			n += n*2.0*river_octavenoise(GetFracDef(6), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += (0.32/n)*river_octavenoise(GetFracDef(6), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		}

		if (n < 0.2){
			n += n*ridged_octavenoise(GetFracDef(5), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		} else {
			n += (0.04/n)*ridged_octavenoise(GetFracDef(5), 
				0.5*octavenoise(GetFracDef(5), 0.5, p), p);
		}
		//smaller ridged mountains
		n += n*0.7*ridged_octavenoise(GetFracDef(5), 
			0.7*octavenoise(GetFracDef(6), 0.6, p), p);

		//n += n*0.7*voronoiscam_octavenoise(GetFracDef(5), 
		//	0.7*octavenoise(GetFracDef(6), 0.6, p), p);

		//n = n*0.6667;
		
		//jagged surface for mountains
		if (n > 0.25) {
			n += (n-0.25)*0.1*octavenoise(GetFracDef(3), 
				Clamp(h*0.0002*octavenoise(GetFracDef(5), 0.6, p),
				 0.5*octavenoise(GetFracDef(3), 0.5, p), 
				 0.6*octavenoise(GetFracDef(4), 0.6, p)), p);
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
		//n = n*0.2;

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

		//if (n < 0.001){
		//	n += n*0.2*ridged_octavenoise(GetFracDef(2), 
		//		0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		//} else if (n <0.01){
		//	n += 0.0002*ridged_octavenoise(GetFracDef(2), 
		//		0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		//} else {
		//	n += (0.01/n)*0.0002*ridged_octavenoise(GetFracDef(2), 
		//		0.5*octavenoise(GetFracDef(2), 0.5, p), p);
		//}

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
 
		n *= 0.3;

	}
	
	n = m_maxHeight*n;
	return (n > 0.0 ? n : 0.0); 
}
