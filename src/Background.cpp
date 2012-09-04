#include "Background.h"
#include "Frame.h"
#include "galaxy/StarSystem.h"
#include "Game.h"
#include "perlin.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/StaticMesh.h"
#include "graphics/Surface.h"
#include "graphics/VertexArray.h"
#include <vector>

using namespace Graphics;

namespace Background
{

Starfield::Starfield(Graphics::Renderer *r)
{
	Init(r);
	//starfield is not filled without a seed
}

Starfield::Starfield(Graphics::Renderer *r, unsigned long seed)
{
	Init(r);
	Fill(seed);
}

Starfield::~Starfield()
{
	delete m_model;
	delete[] m_hyperVtx;
	delete[] m_hyperCol;
}

void Starfield::Init(Graphics::Renderer *r)
{
	// reserve some space for positions, colours
	VertexArray *stars = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE, BG_STAR_MAX);
	m_model = new StaticMesh(POINTS);
	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_STARFIELD;
	desc.vertexColors = true;
	m_material.Reset(r->CreateMaterial(desc));
	m_material->emissive = Color::WHITE;
	m_model->AddSurface(new Surface(POINTS, stars, m_material));

	m_hyperVtx = 0;
	m_hyperCol = 0;
}

void Starfield::Fill(unsigned long seed)
{
	VertexArray *va = m_model->GetSurface(0)->GetVertices();
	va->Clear(); // clear if previously filled
	// Slight colour variation to stars based on seed
	MTRand rand(seed);

	//fill the array
	for (int i=0; i<BG_STAR_MAX; i++) {
		float col = rand.Double(0.3,1.0);
		float colr = pow(col,float(12.0));

		// this is proper random distribution on a sphere's surface
		const float theta = float(rand.Double(0.0, 2.0*M_PI));
		const float u = float(rand.Double(-1.0, 1.0));
		
		vector3f coords = vector3f(
				1000.0f * sqrt(1.0f - u*u) * cos(theta),
				1000.0f * u,
				1000.0f * sqrt(1.0f - u*u) * sin(theta));

		float starId = 1.0;
		// Set star id for use in twinkling and send via alpha, if shaders are enabled
		if (AreShadersEnabled()){	
			const float x = 10.0/100000.0;
			starId = abs((coords+vector3f(1001.0,1001.0,1001.0)).Dot(vector3f(x,x,x)));
		}

		va->Add(coords, Color(colr, col, col, starId));
	}
}
 
//Calculates starfield brightness and star twinkling amount.
//The way this works is that brightness and twinkling are calibrated to look fine on Earth.
//After that the way the brightness and twinkling change under different conditions
//is calculated by mapping the way quantities change due to factors relative to Earth.
void Starfield::CalcParameters(const Camera *camera)
{

	double light = 1.0; // light intensity relative to earths
	// values to use if twinkling is not applicable
	double twinkling = false;
	double brightness = 1.0;
	double time = 0.0;
	double effect = 0.0;
	
	if (camera && camera->GetFrame()->m_parent) {
		const Frame *f = camera->GetFrame()->m_parent;

		//check if camera is near a planet
		Body *camParentBody = f->GetBodyFor();
		if (f->GetBodyFor() && f->GetBodyFor()->IsType(Object::PLANET)) {

			std::vector<Camera::LightSource> l = camera->GetLightSources();
			if (l[0].GetBody() && l.size() == 1){ // check if light is not an artificial light in systems without lights

				for (std::vector<Camera::LightSource>::iterator i = l.begin(); i != l.end(); ++i) {
					const SystemBody *sbody = i->GetBody()->GetSystemBody();

					vector3d position = i->GetBody()->GetInterpolatedPositionRelTo(f->GetBodyFor());
					double distance = position.Length()/AU; // in AU

					//light intensity proportional to: T^4 (see boltzman's law formula Power=Area sigma T^4), r^2 (area = 4 pi r^2), 1/(r^2) (attenuation with inverse square law)
					// as a multiple of sunlight on earths' surface
					double light_ = pow(double(sbody->averageTemp)/5700.0,4.0)*(pow(double(sbody->GetRadius()/SOL_RADIUS),2.0)/pow((distance/1.0),2.0)); // distance in AU
					if ((light_ >= 0.25) &&(light_<=1.0)) 
						light_ = 1.0; //if light is in medium range increase as stars are still dark

					double sunAngle = position.Normalized().Dot(-(f->GetBodyFor()->GetInterpolatedPositionRelTo(camera->GetFrame()).Normalized()));

					if (sunAngle > 0.25) sunAngle = 1.0;
					else if ((sunAngle <= 0.25)&& (sunAngle >= -0.8)) sunAngle = ((sunAngle+0.08)/0.33);
					else /*if (sunAngle < -0.8)*/ sunAngle = 0.0;

					light += light_*sunAngle;
				}

				SystemBody *s = f->GetSystemBodyFor();
				double height = (f->GetBodyFor()->GetPositionRelTo(camera->GetFrame()).Length());

				double pressure, density; 
				static_cast<Planet*>(f->GetBodyFor())->GetAtmosphericState(height,&pressure, &density);

				Color c; double surfaceDensity;
				s->GetAtmosphereFlavor(&c, &surfaceDensity);

				// approximate optical thickness fraction as fraction of density remaining relative to earths
				double opticalThicknessFraction = 1.0-(surfaceDensity-density)/surfaceDensity;
				// tweak optical thickness curve - lower exponent ==> higher altitude before stars show
				opticalThicknessFraction = pow(std::max(0.00001,opticalThicknessFraction),0.15); //max needed to avoid 0^power
				// brightness depends on optical depth and intensity of light from all the stars
				brightness = Clamp(1.0-(opticalThicknessFraction*light),0.0,1.0);

				time = Pi::game->GetTime()*20.0*1e-5; // should be time at frame 
				time = float((time-floor(time))*1e5);

				double temperature = double(s->averageTemp);

				double radiusRatio = (s->GetRadius()/EARTH_RADIUS);

				// set the amount of twinkling to decrease with height and be proportional to temp, rad, and surf den
				effect = Clamp(pow((1.0-(std::min(height-s->GetRadius(),3000.0*radiusRatio)/(3000.0*radiusRatio))),0.3)
					*(surfaceDensity/1.0)*(temperature/EARTH_AVG_SURFACE_TEMPERATURE),0.0,1.0);

				if ((effect > 0.05) && (brightness > 0.1)){// turn on twinkling if effect is large enough 
					twinkling = 1;
				}

			}
		}
	}


	// scale size of stars relative to conditions of calibration
	double starScaling;

	// if a camera exists use camera properties 
	// otherwise use screen properties
	if (camera) 
		starScaling = (camera->GetWidth())/1024.0;
	else 
		starScaling = (Pi::GetScrWidth())/1024.0;
	starScaling = std::max(starScaling,1.0);

	m_starfieldParams.twinkling = twinkling;
	m_starfieldParams.brightness = brightness;
	m_starfieldParams.time = time;
	m_starfieldParams.effect = effect;
	m_starfieldParams.starScaling = starScaling;
		
}

void Starfield::Draw(Graphics::Renderer *renderer,const Camera *camera)
{
	// XXX would be nice to get rid of the Pi:: stuff here
	if (!Pi::game || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
		
		if (AreShadersEnabled()) {
			m_material->specialParameter0 = &m_starfieldParams;
		} else {
			m_material->emissive = Color(m_starfieldParams.brightness);
		}
		//renderer->SetBlendMode(BLEND_ALPHA); remove
		renderer->DrawStaticMesh(m_model);
		//renderer->SetBlendMode(BLEND_SOLID); remove
	} else {
		matrix4x4d m;
		Pi::player->GetRotMatrix(m);
		m = m.InverseOf();
		vector3d pz(m[2], m[6], m[10]); //back vector

		// roughly, the multiplier gets smaller as the duration gets larger.
		// the time-looking bits in this are completely arbitrary - I figured
		// it out by tweaking the numbers until it looked sort of right
		double mult = 0.0015 / (Pi::player->GetHyperspaceDuration() / (60.0*60.0*24.0*7.0));

		double hyperspaceProgress = Pi::game->GetHyperspaceProgress();

		//XXX this is a lot of lines
		if (m_hyperVtx == 0) {
			m_hyperVtx = new vector3f[BG_STAR_MAX * 2];
			m_hyperCol = new Color[BG_STAR_MAX * 2];
		}
		VertexArray *va = m_model->GetSurface(0)->GetVertices();
		for (int i=0; i<BG_STAR_MAX; i++) {

			vector3f v(va->position[i]);
			v += vector3f(pz*hyperspaceProgress*mult);

			m_hyperVtx[i*2] = va->position[i] + v;
			m_hyperCol[i*2] = va->diffuse[i];

			m_hyperVtx[i*2+1] = v;
			m_hyperCol[i*2+1] = va->diffuse[i];
		}
		renderer->DrawLines(BG_STAR_MAX*2, m_hyperVtx, m_hyperCol);
	}
}


MilkyWay::MilkyWay(Graphics::Renderer *r)
{
	m_model = new StaticMesh(TRIANGLE_STRIP);

	//build milky way model in two strips (about 256 verts)
	//The model is built as a generic vertex array first. The renderer
	//will reprocess this into buffered format as it sees fit. The old data is
	//kept around as long as StaticMesh is alive (needed if the cache is to be regenerated)

	VertexArray *bottom = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE);
	VertexArray *top = new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE);

	const Color dark(0.f);
	const Color bright(0.05f, 0.05f, 0.05f, 0.05f);

	//bottom
	float theta;
	for (theta=0.0; theta < 2.f*float(M_PI); theta+=0.1f) {
		bottom->Add(
				vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
				dark);
		bottom->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
	}
	theta = 2.f*float(M_PI);
	bottom->Add(
		vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
		dark);
	bottom->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	//top
	for (theta=0; theta < 2.f*float(M_PI); theta+=0.1f) {
		top->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
		top->Add(
			vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
			dark);
	}
	theta = 2.f*float(M_PI);
	top->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	top->Add(
		vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
		dark);

	Graphics::MaterialDescriptor desc;
	desc.vertexColors = true;
	m_material.Reset(r->CreateMaterial(desc));
	//This doesn't fade. Could add a generic opacity/intensity value.
	m_model->AddSurface(new Surface(TRIANGLE_STRIP, bottom, m_material));
	m_model->AddSurface(new Surface(TRIANGLE_STRIP, top, m_material));
}

MilkyWay::~MilkyWay()
{
	delete m_model;
}

void MilkyWay::Draw(Graphics::Renderer *renderer)
{
	assert(m_model != 0);
	m_material->emissive = Color(m_brightness);
	renderer->DrawStaticMesh(m_model);
}

Container::Container(Graphics::Renderer *r)
: m_milkyWay(r)
, m_starField(r)
{
}

Container::Container(Graphics::Renderer *r, unsigned long seed)
: m_milkyWay(r)
, m_starField(r)
{
	Refresh(seed);
};

void Container::Refresh(unsigned long seed)
{
	// redo starfield, milkyway stays normal for now
	m_starField.Fill(seed);
}

void Container::Draw(Graphics::Renderer *renderer, const matrix4x4d &transform,const Camera *camera)
{
	// Calculate starfield parameters and set milkyway intensity
	Starfield &starfield = static_cast<Starfield>(m_starField);
	starfield.CalcParameters(camera);
	const Starfield::StarfieldParameters &sp = starfield.GetStarfieldParams();
	MilkyWay &milkyway = static_cast<MilkyWay>(m_milkyWay);
	milkyway.SetIntensity(sp);

	//XXX not really const - renderer can modify the buffers
	glPushMatrix();
	renderer->SetBlendMode(BLEND_SOLID);
	renderer->SetDepthTest(false);
	renderer->SetTransform(transform);
	milkyway.Draw(renderer);
	// squeeze the starfield a bit to get more density near horizon
	matrix4x4d starTrans = transform * matrix4x4d::ScaleMatrix(1.0, 0.4, 1.0);
	renderer->SetTransform(starTrans);
	starfield.Draw(renderer, camera);
	renderer->SetDepthTest(true);
	glPopMatrix();
}

}; //namespace Background
