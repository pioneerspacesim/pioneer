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
				pp[1+x][1+y][3] = (pp[1+x][1+y][3]*m_roughnessScaling[2])+m_roughMin[2];
				pp[1+x][1+y][2] = (pp[1+x][1+y][2]*m_roughnessScaling[1])+m_roughMin[1];
				pp[1+x][1+y][1] = (pp[1+x][1+y][1]*m_roughnessScaling[0])+m_roughMin[0];
				pp[1+x][1+y][0] = (pp[1+x][1+y][0]*m_heightScaling);//+m_minh;

				if (i<10000&&(pp[0][0][0] != 44.3750||pp[0][0][0] !=0)){

				//printf("hmap (%i,%i) = %f,%i",iy+1,ix+1,p[0][0][0],*(Uint16*)hrmappoint);
				i++;
				};
			}
		}
		
	
		//{
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


			//v[0] has height, v[1] to v[3] has roughness
			//terrain code here
			v[0]/=m_planetRadius;
			//double h = v[0]/(double(2^32)*m_heightScaling);//expression for height on a scale of 0 to 1
			v[0] = (v[0]<0 ? 0 : v[0]);
			return v[0];
			//(v[0])/30.0;//log10(log10(v[0]+10)*200+10)*100000-1000;

	
}