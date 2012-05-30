#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
template <>
const char *TerrainHeightFractal<TerrainHeightMapped2>::GetHeightFractalName() const { return "Mapped2"; }

template <>
TerrainHeightFractal<TerrainHeightMapped2>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightMapped2>::GetHeight(const vector3d &p)
{

	double latitude = -asin(p.y);
	if (p.y < -1.0) latitude = -0.5*M_PI;
	if (p.y > 1.0) latitude = 0.5*M_PI;
//	if (!isfinite(latitude)) {
//		// p.y is just n of asin domain [-1,1]
//		latitude = (p.y < 0 ? -0.5*M_PI : M_PI*0.5);
//	}
	double longitude = atan2(p.x, p.z);
	double px = (((m_heightMapSizeX-1) * (longitude + M_PI)) / (2*M_PI));
	double py = ((m_heightMapSizeY-1)*(latitude + 0.5*M_PI)) / M_PI;
	int ix = int(floor(px));
	int iy = int(floor(py));
	ix = Clamp(ix, 0, m_heightMapSizeX-1);
	iy = Clamp(iy, 0, m_heightMapSizeY-1);
	double dx = px-ix;
	double dy = py-iy;

	// p0,3 p1,3 p2,3 p3,3
	// p0,2 p1,2 p2,2 p3,2
	// p0,1 p1,1 p2,1 p3,1
	// p0,0 p1,0 p2,0 p3,0
	double map[4][4];
	for (int x=-1; x<3; x++) {
		for (int y=-1; y<3; y++) {
			map[x+1][y+1] = m_heightMapScaled[Clamp(iy+y, 0, m_heightMapSizeY-1)*m_heightMapSizeX + Clamp(ix+x, 0, m_heightMapSizeX-1)];
		}
	}

	double c[4];
	for (int j=0; j<4; j++) {
		double d0 = map[0][j] - map[1][j];
		double d2 = map[2][j] - map[1][j];
		double d3 = map[3][j] - map[1][j];
		double a0 = map[1][j];
		double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
		double a2 = 0.5*d0 + 0.5*d2;
		double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
		c[j] = a0 + a1*dx + a2*dx*dx + a3*dx*dx*dx;
	}

	{
		double d0 = c[0] - c[1];
		double d2 = c[2] - c[1];
		double d3 = c[3] - c[1];
		double a0 = c[1];
		double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
		double a2 = 0.5*d0 + 0.5*d2;
		double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
		double v = 0.1 + a0 + a1*dy + a2*dy*dy + a3*dy*dy*dy;
	
		//v = (v<0 ? 0 : v);
	
		v=v*m_heightScaling+m_minh; // v = v*height scaling+min height
		v/=m_planetRadius;

		v += 0.1;
		double h = 1.5*v*v*v*ridged_octavenoise(16, 4.0*v, 4.0, p);
		h += 30000.0*v*v*v*v*v*v*v*ridged_octavenoise(16, 5.0*v, 20.0*v, p);
		h += v;
		h -= 0.09;

		return (h > 0.0 ? h : 0.0);

	}

}

