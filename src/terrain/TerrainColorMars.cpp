#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;

template <>
const char *TerrainColorFractal<TerrainColorMars>::GetColorFractalName() const { return "Mars"; }

template <>
TerrainColorFractal<TerrainColorMars>::TerrainColorFractal(const SystemBody *body) : Terrain(body)
{
}

template <>
vector3d TerrainColorFractal<TerrainColorMars>::GetColor(const vector3d &p, double height, const vector3d &norm)
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
			pp[1+x][1+y][3] = pow(10.0,(pp[1+x][1+y][3]*m_roughnessScaling[2]+m_roughMin[2]+log10(0.49e3)));
			pp[1+x][1+y][2] = pow(10.0,(pp[1+x][1+y][2]*m_roughnessScaling[1]+m_roughMin[1]+log10(2.4e3)));
			pp[1+x][1+y][1] = pow(10.0,(pp[1+x][1+y][1]*m_roughnessScaling[0]+m_roughMin[0]+log10(9.2e3)));
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

	//v[0] has height, v[1] to v[3] has roughness

	// roughness and height
	v[3]; // 0.6 km
	v[2]; // 2.4 km
	v[1]; // 9.6 km
	v[0]; // height

	double n = m_invMaxHeight*height/2;
	height *= m_planetRadius;
	const double flatness = pow(p.Dot(norm), 6.0);
	const vector3d color_cliffs = m_rockColor[1];

	// Mars with the poles have been replaced and a simple colour added.

#define colour(a,b,c) (vector3d(a/255.0,b/255,c/255.0)) 
//1. light yellowish full mars map www.solarviews.com/raw/mars/marscyl1.jpg
//2. darker reddish full mars map planetpixelemporium.com

#define lightcol1 colour(240.0,139.0,104.0)
#define lightcol2 colour(230.0,145.0,44.0)
#define lightroughcol1 colour(136.0,83.0,79.0)
#define lightroughcol2 colour(127.0,98.0,66.0)
#define darkcol1 colour(146.0,89.0,54.0)
#define darkcol2 colour(177.0,132.0,68.0)
#define darkroughcol1 colour(72.0,57.0,62.0)
#define darkroughcol2 colour(63.0,58.0,64.0)

	vector3d col,col_light, col_dark;

	// go from reds to yellows for the Tharsis highlands
	const double c1 = 25000.0;
	double i1;

	if (height > c1){
		//col = interpolate_color(equatorial_desert, vector3d(.8,.75,.5), vector3d(.52, .5, .3));
		i1 = (height-c1)*(1.0/(28000.0-c1));
	}

	col_light = lightcol1;//interpolate_color(i1, lightcol1, lightcol2);	
	col_dark = darkcol1; //interpolate_color(i1, darkcol1, darkcol2);
	//col = col_light;
	col = interpolate_color(flatness, col_dark, col_light);
	
	double v1 = v[1]+v[2]+v[3];
	v1*=1e4/3.0;
	v1 = std::min(v1,3.0);
	v1 = v1/(1.5+v[1]);
	
	col = interpolate_color(v1, col_light, col_dark);

	//else col = vector3d(0.5,0.5,0.5);
	

	//special color regions
	double cr=0.0;
	double cc = 0.0; 
	double la[3],lo[3],lawidth[3],lowidth[3];

	// google earth (paths with ruler and readout in decimal degrees)
	// regions form a box
	double lat = latitude+0.5*M_PI; // 0 at south pole to 180 at north pole
	double lon = longitude+M_PI; // 0 to 360
	double blend = 0.0,blend2 = 0.0;

/*
	// Ridges near olympus mons 1 
	//coord - min coord
	la[0] = lat - DEG2RAD(23.6+90.0-58.5);// -58.5 needed to align with pioneer 
	lo[0] = lon - DEG2RAD(-152.6+180.0+180.0); // -180 = 0
	//max value - min value 
	lawidth[0] = DEG2RAD(34.1-23.6);
	lowidth[0] = DEG2RAD(22.6);

	// Ridges near Olympus mons 2 (smaller)
	//coord - min coord
	la[1] = lat - DEG2RAD(35.2+90.0-58.5);// -58.5 needed to align with pioneer 
	lo[1] = lon - DEG2RAD(-151.0+180.0+180.0); // -180 = 0
	//max value - min value 
	lawidth[1] = DEG2RAD(11.0);
	lowidth[1] = DEG2RAD(10.0);
*/
	//valles marineris
	//1.5, -49.9, -24.9(ok down to -16), -105.0
	la[2] = lat - DEG2RAD(-24.9+90.0+28.0);// -58.5 needed to align with pioneer 
	lo[2] = lon - DEG2RAD(-105.0+180.0+180.0); // +180 needed to align
	//max value - min value 
	lawidth[2] = DEG2RAD(13.4);
	lowidth[2] = DEG2RAD(55.1);



// blending might not be the best way for colours, or it might involve selection of colour algorithms based on noise multiplied by blends
#define	COMPUTEBLENDVAL(idx,blendlimit) \
	/*// set blend 0-1 as coords approach the last 0.05 of the width*/\
	double b = std::max(abs((la[idx]/lawidth[idx]) - 0.5),abs((lo[idx]/lowidth[idx]) - 0.5));\
	b = (b< blendlimit)?1.0:(0.5-b)*(1.0/(0.5-blendlimit));


#define CHECKIFWITHINREGION(idx) ((la[idx]>0.0)&&(lo[idx]>0.0)&& la[idx] < lawidth[idx] && lo[idx] < lowidth[idx])
/*
	// Ridges 
	if (CHECKIFWITHINREGION(0) 
		)
	{
		COMPUTEBLENDVAL(0,0.45);
		blend = b;
		//cr
	}

	// Ridges 2
	if (CHECKIFWITHINREGION(1) )
	{
		COMPUTEBLENDVAL(1,0.45);
		//blend = b;
		//cr
	}

*/
	// valles marineris - split up into two smaller areas if it includes too much of surrounding areas
	if (CHECKIFWITHINREGION(2))
	{
		COMPUTEBLENDVAL(2,0.35);
		//blend2 = b;
		
		// darken terrain with decreasing height in the valley area
		// mask area based on height
		double b2 = TRANSITION(height,20000.0,25000.0);
		// mask trough based on high roughness
		double b3 = TRANSITION(v[1],4e-5,5e-5);
		b3 =0.2*b2*b*b3;

		vector3d col1 = vector3d(1.0,1.0,1.0);
		col = interpolate_color(b3, col, darkcol1);
	}



	// south pole - height and roughness indicate troughs not hills
	vector3d spole = vector3d(0.0,-1.0,0.0).Normalized(); // ice is to one side of the pole, the other side called the 'cryptic region'

	// sp varies from 1 at p to 0 perpendicular to p to -1 at opposite pole
	double sp = spole.Dot(p);

	// include in region if within a x degree radius of the point p 
	if (sp > cos(DEG2RAD(10.0))){

		double b3 = 1.0-((1.0-sp)/(1.0-cos(DEG2RAD(10.0))));;
		b3 = TRANSITION(b3,0.89,1.0);

		// ice cap
		vector3d scap = vector3d(0.02625,-0.94,-0.03125).Normalized();
		double sc = scap.Dot(p);
		double b1=0.0,ccap=0.0;

		// include in region if within a x degree radius of the point p 
		if (sc > cos(DEG2RAD(3.0))){
			b1 = 1.0-((1.0-sc)/(1.0-cos(DEG2RAD(3.0))));
			//b1 = TRANSITION(b1,0.7,1.0);
			// ice cap fractals
			double ice = 0.0;		

			double vv = 10300.0;
			// calc ice-ground transition. ground color at 0 ice at 1.0
			ice = (1.0-TRANSITION(v[1],0.30e-4,0.31e-4)) // ground colour with roughness
				*TRANSITION(v[0]+400*b3,(9800.0-5.0),9800.0) // excude areas to avoid non circular shape
				*((b3>0.55)?(1.0-TRANSITION(-(height*m_planetRadius-v[0])-0.0,4.9460e0,4.9461e0)):1.0) // ground colour in low lying areas near the pole circle
				*flatness;
			vector3d c = interpolate_color(ice, col, vector3d(1,1,1));
			//printf("height %f",(height*m_planetRadius));
			return c;
		}

		// calculate pole fractals here
		//double cpole = 0.0;
		//cpole = 0.01;
		//calculate icecap to pole transition
		//cpole = cpole*(1.0-b1)+ccap*b1;
		// pole to general terrain
		//double b2 = 1.0-((1.0-sp)/(1.0-cos(DEG2RAD(10.0))));;
		//b2 = (b2< 0.25)?b2*4.0:1.0;
		//hr = hpole;
		//blend = b2;

	}
	
	// n pole
	vector3d npole = vector3d(0.0,1.0,0.0).Normalized();
	double np = npole.Dot(p); 

	// include in region if within a x degree radius of the point p 
	if (np > cos(DEG2RAD(11.0))){
		// pole to general terrain
		double b2 = 1.0-((1.0-np)/(1.0-cos(DEG2RAD(10.0))));;
		double b3 = TRANSITION(b2,0.2,1.0);
		// calc ice-ground transition. ground color at 0 ice at 1.0
		double ice = (1.0-TRANSITION(v[1],0.20e-4,0.21e-4)) // ground colour with roughness
				*TRANSITION(v[0]+0.0*400*b3,(1700.0-1.0),1800.0) // excude areas to avoid non circular shape
				
				*flatness;
		vector3d c = interpolate_color(ice, col, vector3d(1,1,1));
		//printf("height %f",v[0]);
		return c;
		// height fractals here
		// double hpole = 0.0;
		//hr = 0.01;
		//blend = b2;
	}

	return col;

	
}