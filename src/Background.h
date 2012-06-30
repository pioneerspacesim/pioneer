#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"

namespace Graphics {
	class Renderer;
	class StaticMesh;
	class Shader;
	class Material;
}
class Camera;

/*
 * Classes to draw background stars and the milky way
 */

namespace Background
{
	class BackgroundElement
	{
	public:
		void SetIntensity(float intensity);

	protected:
		RefCountedPtr<Graphics::Material> m_material;
	};

	class Starfield : public BackgroundElement
	{
	public:
		//does not Fill the starfield
		Starfield();
		Starfield(unsigned long seed);
		~Starfield();
		void Draw(Graphics::Renderer *r, Camera *camera,
			int twinkling, double time, double effect,
			int fade, vector3d upDir, double darklevel, double maxSunAngle);
		void Starfield::CalcParameters(Camera *camera, Frame *f,
			double &brightness, int &twinkling, double &time, double &effect,
			int &fade, vector3d &upDir, double &darklevel, double &maxSunAngle);
		//create or recreate the starfield
		void Fill(unsigned long seed);

	private:
		void Init();
		static const int BG_STAR_MAX = 10000;
		Graphics::StaticMesh *m_model;
		ScopedPtr<Graphics::Shader> m_shader;

		//hyperspace animation vertex data
		//allocated when animation starts and thrown away
		//when starfield is destroyed (on exiting hyperspace)
		vector3f *m_hyperVtx;
		Color *m_hyperCol;
	};
	
	class MilkyWay : public BackgroundElement
	{
	public:
		MilkyWay();
		~MilkyWay();
		void Draw(Graphics::Renderer *r);

	private:
		Graphics::StaticMesh *m_model;
		ScopedPtr<Graphics::Shader> m_shader;
	};

	// contains starfield, milkyway, possibly other Background elements
	class Container
	{
	public:
		// default constructor, needs Refresh with proper seed to show starfield
		Container();
		Container(unsigned long seed);
		void Draw(Graphics::Renderer *r, const matrix4x4d &transform, Camera *camera,
			int twinkling, double time, double effect, int fade, vector3d upDir, double darklevel, double maxSunAngle) const;
		void Refresh(unsigned long seed);
		void Container::CalcParameters(Camera *camera, Frame *f, double &brightness,int &twinkling, double &time, double &effect, int &fade, vector3d &upDir, double &darklevel, double &maxSunAngle);
		void SetIntensity(float intensity);

	private:
		Starfield m_starField;
		MilkyWay m_milkyWay;
	};

}; //namespace Background

#endif
