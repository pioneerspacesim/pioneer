#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
template <>
const char *TerrainHeightFractal<TerrainHeightMapped3>::GetHeightFractalName() const { return "Mapped3"; }

template <>
TerrainHeightFractal<TerrainHeightMapped3>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
double TerrainHeightFractal<TerrainHeightMapped3>::GetHeight(const vector3d &p)
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

	double pp[4][4][4];
	for (int x=-1; x<3; x++) {
		for (int y=-1; y<3; y++) { 

			const int xx = Clamp(x+ix,0,(m_heightMapSizeX+3)-1)*4;
			const int offset = Clamp(y+iy, 0, (m_heightMapSizeY+3)-1)*(m_heightMapSizeX+3)*4 +xx;
			Uint16* hrmappoint = m_heightMapScaled+offset;

			pp[1+x][1+y][0] = hrmappoint[0];
			pp[1+x][1+y][1] = hrmappoint[1];
			pp[1+x][1+y][2] = hrmappoint[2];
			pp[1+x][1+y][3] = hrmappoint[3];
			static int i = 0;


			// convert from map format
			pp[1+x][1+y][3] = (pp[1+x][1+y][3]*m_roughnessScaling[2]);//+m_roughMin[2]; // min is ignored to get values starting from 0
			pp[1+x][1+y][2] = (pp[1+x][1+y][2]*m_roughnessScaling[1]);//+m_roughMin[1];
			pp[1+x][1+y][1] = (pp[1+x][1+y][1]*m_roughnessScaling[0]);//+m_roughMin[0];
			pp[1+x][1+y][0] = (pp[1+x][1+y][0]*m_heightScaling);//+m_minh;

		}
	}



	double c[4][4];
	for (int jj = 0; jj < 4; jj++){	// to do - sse
		for (int j=0; j<4; j++) {
			double d0 = pp[0][j][jj] - pp[1][j][jj];
			double d2 = pp[2][j][jj] - pp[1][j][jj];
			double d3 = pp[3][j][jj] - pp[1][j][jj];
			double a0 = pp[1][j][jj];
			double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
			double a2 = 0.5*d0 + 0.5*d2;
			double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
			c[j][jj] = a0 + a1*dx + a2*dx*dx + a3*dx*dx*dx;
		}
	}

	double v[4];

	for (int j=0; j<4; j++){
		double d0 = c[0][j] - c[1][j];
		double d2 = c[2][j] - c[1][j];
		double d3 = c[3][j] - c[1][j];
		double a0 = c[1][j];
		double a1 = -(1/3.0)*d0 + d2 - (1/6.0)*d3;
		double a2 = 0.5*d0 + 0.5*d2;
		double a3 = -(1/6.0)*d0 - 0.5*d2 + (1/6.0)*d3;
		v[j] = a0 + a1*dy + a2*dy*dy + a3*dy*dy*dy;
	}

	// 2^16*scaling gives range of the desired quantity, add to min value to get maximum. easy to print out and check
	// e.g. height has a range of 44.224 km a lot of it due to tharsis bulge and olympus mons 
	// generally it's around 0.005 radius (16 km) with variation of +-0.002 (7 km) (very rougly speaking)

	//v[0] has height, v[1] to v[3] has roughness
	//terrain code here

	// scale roughness and height by planet radius
	v[3]/=m_planetRadius; // 0.6 km
	v[2]/=m_planetRadius; // 2.4 km
	v[1]/=m_planetRadius; // 9.6 km
	v[0]/=m_planetRadius; // height

	// Value Ranges:
	// 1. roughness data 0.6,2.4,9.6 km:  mostly 0.0 +-0.000003 i.e. +-9m given radius of 3376m
	// 2. height data : generally 0.005 +- 0.002 (16km+-7km) Limits: 0.0-44.225km

	double h = v[0];
	h = 0.0;

	//1. map has details roughly upto spatial frequency (jizm in the code) 1/avg grid displacement 
	// map size: x (longitude) 2881, y (latitude) 1441 . Avg grid displacement (radii) ~= degrees per point/360 * 2pi r = (360/2881)/360*2pi*1.0 radii = 0.00275
	// Max spatial frequency (jizm) in heightmap = 1/avg grid displacement = 363.2 
	// octaves in height map = log base 2 (363.2) = 8.5 (can go easy on the first 8 octaves - unless part of additional features layered etc.)
	

	//2. spatial freqs 1/9200 to 1/2400 m corresponding to 9.2km to 2.4km 
	// jizm from m_planetRadius/9200(369.1) to m_planetRadius/9200 (1415) ==> 3 octaves 	
	// 9.2km spatial freq 369.1 is basically at the scale of the grid (363.2)

	//3. spatial freqs 1/2400 to 1/600 corresponding to 2.4km to 0.6km 
	// jizm from m_planetRadius/2400 (1415) to m_planetRadius/600 (5660) ==> 3 octaves

	// 4. the roughness data (range 0.0+-0.000003) needs to be used to manipulate fractals
	

	const double r1 = (v[1]+0.3*v[2]); // measure of roughness to affect fractals

	h = r1*r1*r1*ridged_octavenoise(10, 4.0, 4.0, 363.2, p);


	//double h = 1.5*r*r*r*ridged_octavenoise(10, 4.0*v[1], 4.0, 363.2, p);
	//h += 30000.0*v*v*v*v*v*v*v*ridged_octavenoise(16, 5.0*v, 20.0*v, p);


	//h = abs(h);

	//h = (h<0 ? 0 : h);

	//printf(" h %f, v[0] %f, v[1] %f,v[2] %f    ",h, v[0],v[1],v[2]);
	//hprintf("range %f, radius %f   ",double(pow(2.0,16.0))*m_heightScaling, m_planetRadius);
	//printf("x: %f,y: %i",double(m_heightMapSizeX),m_heightMapSizeY);

	return v[0];
	//(v[0])/30.0;//log10(log10(v[0]+10)*200+10)*100000-1000;


}

