#include "Terrain.h"
#include "TerrainNoise.h"

using namespace TerrainNoise;
template <>
const char *TerrainHeightFractal<TerrainHeightMapped3>::GetHeightFractalName() const { return "Mapped3"; }

template <>
TerrainHeightFractal<TerrainHeightMapped3>::TerrainHeightFractal(const SystemBody *body) : Terrain(body)
{
	//small fractal/high detail
	//SetFracDef(2, m_maxHeightInMeters*0.00000000001, 50, 50*m_fracmult);
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
	//terrain code here

	// scale roughness and height by planet radius
	v[3]/=m_planetRadius; // 0.6 km
	v[2]/=m_planetRadius; // 2.4 km
	v[1]/=m_planetRadius; // 9.6 km
	v[0]/=m_planetRadius; // height

	// Value Ranges:
	// 1. roughness data 0.6,2.4,9.6 km: mostly -1e-5 to + 5e-5 Limits 0 to 6.3e-4
	// 2. height data : generally 0.005 +- 0.002 (16km+-7km) Limits: 0.0-44.225km

	//Roughness data*scale length in m: 0 to 6.3e-4, note roughness is normalised by radius later (Freemat output)
	//Fraction of  0.1       0.2       0.3       0.4       0.5       0.6       0.7       0.8       0.9    0.9       0.925     0.95      0.975     1.0
    //values 
	//9.2km(1e-4)* 0.0356    0.0619    0.0877    0.1179    0.1621    0.2234    0.3228    0.4972    0.9372 0.9372    1.1796    1.5317    2.2311    6.3105
	//2.4km        0.0578    0.0972    0.1330    0.1697    0.2185    0.2884    0.3889    0.5590    0.9166 0.9166    1.0859    1.3741    1.9919    6.3105
	//0.6km        0.0717    0.1050    0.1374    0.1733    0.2164    0.2746    0.3524    0.4945    0.7683 0.7683    0.8880    1.0586    1.4049    6.3105 


	double h = v[0];
	h = 0.0;

	//1. map has details roughly upto spatial frequency (jizm in the code) 1/avg grid displacement 
	// map size: x (longitude) 2881, y (latitude) 1441 . Avg grid displacement (radii) ~= degrees per point/360 * 2pi r = (360/2881)/360*2pi*1.0 radii = 0.00275 radii = 7406.2 m
	// Max spatial frequency (jizm) in heightmap = 1/avg grid displacement = 363.2 
	// octaves in height map = log base 2 (363.2) = 8.5 (can go easy on the first 8 octaves - unless part of additional features layered etc.)
	

	//2. spatial freqs 1/9200 to 1/2400 m corresponding to 9.2km to 2.4km 
	// jizm from m_planetRadius/9200(369.1) to m_planetRadius/9200 (1415) ==> 3 octaves 	
	// 9.2km spatial freq 369.1 is basically at the scale of the grid (363.2)

	//3. spatial freqs 1/2400 to 1/600 corresponding to 2.4km to 0.6km 
	// jizm from m_planetRadius/2400 (1415) to m_planetRadius/600 (5660) ==> 3 octaves


	// 4. the roughness data (range 0 to 6.3e-4) needs to be used to manipulate fractals
	
	// 5. number of octaves needed to go from heightmap grid to 1m = log base 2 (7406) = 12.8


	// Possible algorithms:
	// Height as a measure of roughness doesn't work - e.g. the tharsis area is 30km+ but is mostly very smooth. cratered terrain in low lying areas can be rough.
	// Use ratios of roughness: ratio of roughness to roughness at a smaller scale. e.g. low roughness at 2.4 km compared to 9.2 might indicate rolling hills
	// Area effects by dotting a location vector with p

	// The roughness data was gathered by taking each of the highres height points within each grid of the map,
	// finding a second order derivative (rate of change of the slope) for height point, gathering the second derivatives in each grid,
	// and finding the range of the mid 50% of the second derivatives within each grid point. 
	// So it's mostly just a measure as it doesn't map directly into fractal properties and must be used to select fractal types and their properties.


	// Isolate special regions

	//special region height
	double hr=0.0;
	double hc = 0.0; //additive height contribution

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
	la[2] = lat - DEG2RAD(-24.9+90.0+15.0);// -58.5 needed to align with pioneer 
	lo[2] = lon - DEG2RAD(-105.0+180.0+180.0); // +180 needed to align
	//max value - min value 
	lawidth[2] = DEG2RAD(26.4);
	lowidth[2] = DEG2RAD(55.1);


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
		//blend = b;
		//hr = 0.01;
	}

	// Ridges 2
	if (CHECKIFWITHINREGION(1) )
	{
		COMPUTEBLENDVAL(1,0.45);
		//blend = b;
		//hr = 0.01;
	}
*/
	// valles marineris - split up into two smaller areas if it includes too much of surrounding areas
	if (CHECKIFWITHINREGION(2) )
	{
		COMPUTEBLENDVAL(2,0.45);
		blend2 = b;
		// terrain code after general terrain code
	}
	
	// n pole
	vector3d npole = vector3d(0.0,1.0,0.0).Normalized();
	double np = npole.Dot(p); 

	// include in region if within a x degree radius of the point p 
	if (np > cos(DEG2RAD(10.0))){
		// pole to general terrain
		double b2 = 1.0-((1.0-np)/(1.0-cos(DEG2RAD(10.0))));;
		b2 = (b2< 0.25)?b2*4.0:1.0;
		// height fractals here
		// double hpole = 0.0;
		//hr = 0.01;
		//blend = b2;
	}



	// main set of fractals
	// if values from these are reused in special regions include the special region after this
	double r1,r2,r3;
	double a1,a2,a3;
	double h1,h2=0.0,h3=0.0,hi1=0.0;

	if (blend < 1.0){ // create another blend if h# is needed for special regions

		// calculate expression for amplitudes
		// use relationship delta h = delta h' * delta x twice on h'' to see delta h is proportional to (delta x)^2 (assuming h(x0), h(x0)' = 0)
		// amplitude should involve roughness (h'') * scale in radii ^2 (delta x ^ 2)
		// roughness data is multiplied by delta x as is
		a1 = 2.5e9*v[1]*pow(1.0/369.1,1.0); // measure of amplitude 
		a2 = 2.5e8*v[2]*pow(1.0/1415.0,1.0);
		a3 = 3.125e8*v[3]*pow(1.0/5660.0,1.0); 

		//measure of roughness - 0 to 1
		// part ratio of roughness between scales to a power and part contributions from each scale
		// Note: s curve x/(a+bx) , compresses 0-inf to 0-1. midpoint is at a

		// map 0-7e4 to 0-1 
		r1 = 1.0*(v[1]/((1e-4/3396000.0)+1.0*v[1]));
		// map 0-1 to around 0.5+-0.2 with 0.45 being mapped to 0.5
		r1 = (4.5/7.0)*r1/(0.45+r1); 

		r2 = 1.0*(v[2]/((1.5e-4/3396000.0)+1.0*v[1]));
		r2 = (5.5/7.0)*r2/(0.4+r2);

		r3 = 1.0*(v[1]/((1e-4/3396000.0)+1.0*v[1]));
		r3 = (5.0/7.0)*r3/(0.7+r3);

		// 3 octaves starting from heightmap grid freq
		// target h values of around 0.1 wavelength=9200m (3e-3 radii) i.e. e-4, max 3e-3
		hi1 = a1*ridged_octavenoise(3, 1.0*r1, 2.0, 363.2/2, p); // max 3e5
		double r12 = std::min(hi1*hi1*2.5e4,0.3);
		h = (hi1+a1*0.75*ridged_octavenoise(3, r12*r12*r12, 2.0, 363.2*4.0, p));

		// 6 octaves starting from heightmap grid freq*2^1
		// target h values of around 0.1 wavelength=2400m (7e-4 radii) i.e. e-5, max 7e-4
		h2 = a2*ridged_octavenoise(3, 1.0*r2, 2.0, 363.2*pow(2.0,1.0), p);

		// 8 octaves starting from heightmap grid freq*2^4
		// target h values of around 0.1 wavelength=600m (2e-4 radii) i.e. 2e-5, max 2e-4
		h3 = a3*ridged_octavenoise(6, 1.0*r3, 2.0, 363.2*pow(2.0,4.0), p);

	}

	/*
	// useful code to check if having extra octaves makes much difference
	// h_low = 0.3*octavenoise(10, r1, 2.0, freq, p);
	// h_high = 0.3*octavenoise(20, r1, 2.0, freq, p);

	if (abs(h_low-h_high) < 0.1*(1.0/(freq*pow(2.0,num octaves=20.0))) ){
		printf (" difference %f\n",abs(h_low-h_high)*m_planetRadius);
	}*/


	// valles marineris - split up into two smaller areas if it includes too much of surrounding areas
	if (CHECKIFWITHINREGION(2) )
	{
		//uses blend 2

		// add ridges high up slopes to simulate erosion
		// roughness - use r1, map 0.5 to 0.5
		double rv = r1/(0.3+r1);
		// calculate max possible amplitude of the ridged octave noise of h1
		double maxah1 = std::max(fabs(1.0-r1+r1*r1+r1*r1*r1),1.0); // 3 octaves
		double av = (((hi1/(a1*maxah1)))); //fraction of max height
		double av_ = 1.0-TRANSITION(av,0.80,0.99); // map 0.9-0.8 to 0-1 and 0.8 to 0.7 to 1-0
		double av__ = TRANSITION(av,0.70,0.80); // map 0.84 to 0.89 to 1-0
		av = pow(av_,0.15)*av__*av__;
		double hv = ridged_octavenoise(5, rv, 2.0, 363.2*1.5, p);
		hv = 0.025*a1*av*pow(hv,15.0-12.0*av_);
		hc = hv;
	}	


	// south pole - height and roughness6indicate troughs not hills
	vector3d spole = vector3d(0.00,-1.0,0.0).Normalized(); // ice is to one side of the pole, the other side called the 'cryptic region'
	// sp varies from 1 at p to 0 perpendicular to p to -1 at opposite pole
	double sp = spole.Dot(p);

	// include in region if within a x degree radius of the point p 
	if (sp > cos(DEG2RAD(10.0))){

		// ice cap
		vector3d scap = vector3d(0.02625,-0.94,-0.03125).Normalized();
		double sc = scap.Dot(p);
		double b1=0.0,hcap=0.0;
		// include in region if within a x degree radius of the point p 
		if (sc > cos(DEG2RAD(3.0))){
			b1 = 1.0-((1.0-sc)/(1.0-cos(DEG2RAD(3.0))));
			b1 = (b1< 0.2)?b1*5.0:1.0;
			// ice cap height fractals
			//hcap =  
		}

		// pole to general terrain
		double b2 = 1.0-((1.0-sp)/(1.0-cos(DEG2RAD(10.0))));;
		double b3 = TRANSITION(b2,0.0,0.2);
		b2 = (b2< 0.25)?b2*4.0:1.0;

		// calculate pole height fractals here
		double hpole = 0.0;
		hpole = -2.0*(h+h2+h3); // when added to ht will invert height fractals

		//calculate icecap to pole transition
		//hpole = hpole*(1.0-b1)+hcap*b1;
		
		hc = hpole;
		blend2 = b3;

	}
	
	//printf("x: %f,y: %i",double(m_heightMapSizeX),m_heightMapSizeY);
	//printf("h %f, v0 %f, v1 %f, v2 %f, v3 %f, r1 %f, a1 %f\n",h,v[0],3396000.0*v[1],3396000.0*v[2],3396000.0*v[3],r1,a1);

	double ht = v[0]+h+h2+h3;
	
	if (blend > 0.0){
		ht = (1.0-blend)*ht+hr*blend;
	}

	// use hc for simple additive height contributions
	if (hc!=0.0){
		ht = ht+hc*blend2;
	}
	
	ht = (ht<0.0 ? 0.0 : ht);

	return ht;


}


// Simple roughness visualisation code for Freemat (Matlab) http://freemat.sourceforge.net/
// The idea is to compare roughness values on different 
// scales and see how they correspond to visible terrain on google earth then use that to select fractals.
// Outputting v[1] to v[3] in get height also works but is slower/non-readable.
// 1. The data was taken from here http://www.planetary.brown.edu/rough/index.html
// 2. 3 Papers on that site have a lot of hints on how roughness corresponds to terrain
// 3. Simplest way: Start freemat, tools-editor, cut/paste matlab code, right click to select blocks of text and execute the bits you want

// Note:roughness data in v[n] is curvature roughhness*scale length

/*
    clear all;  %clear all variables
    hmap_ = zeros(1440, 2880);

    % load 3 roughness maps for different baseline scales
    f06 = fopen(['C:\Download\E8R0.RAW'],'r','b');
    f24 = fopen(['C:\Download\E8R1.RAW'],'r','b');
    f92 = fopen(['C:\Download\E8R2.RAW'],'r','b');
    
    
    %[rmappic,colmap1,alpha] = imread('C:\Download\E8R.TIF')
    
    
    rmap06 = fread(f06,fliplr(size(hmap_)),'uint8');rmap24 = fread(f24,fliplr(size(hmap_)),'uint8');rmap92 = fread(f92,fliplr(size(hmap_)),'uint8');
    fclose all;
    %results have columns and rows interchanged, take the transpose next operation 
    
    %convert to numerical values
    rmap06 = (rmap06'-511)/80;rmap24 = (rmap24'-511)/80;rmap92 = (rmap92'-511)/80;
    
    %this is what v[1] to v[3] holds currently
    rmap06c = 10.^rmap06; rmap24c = 10.^rmap24;rmap92c = 10.^rmap92;
    
    %code for displaying images
    figure(1); %change firgure number to open and select more figure windows
    image(rmap92c); %display 2d matrix as image. change rmap name
    colorbar; %add a scale
    colormap(gray); %change the colours if needed, copper is another  
    %induvidual values can be read out by clicking the sample button
    
    %print out what values are at what fraction of data (histogram) and display it
    rmap = rmap92c; %change to correct rmap
    n=hist(reshape(rmap,[1,numel(rmap)]),[0.5:1:1000]*7e-7); 
    figure(5);
    n=cumsum(n)./numel(rmap); 
    plot([0.5:1:1000]*7e-7,n);
    %print out histogram in steps of 0.1
    interplin1(n,[0.5:1:1000]*7e-7,[0.1:0.1:0.9,0.9:0.025:1.0])
    
    %plot s-curve     
    figure(6); x=[0:0.1:7]*1e-4;plot(x,x./(1e-4+x));

*/