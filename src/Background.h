#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"

namespace Graphics {
	class Renderer;
	class StaticMesh;
	class Shader;
}

/*
 * Classes to draw background stars and the milky way
 */

namespace Background
{
	class Starfield
	{
	public:
		//does not Fill the starfield
		Starfield();
		Starfield(unsigned long seed);
		~Starfield();
		void Draw(Graphics::Renderer *r);
		//create or recreate the starfield
		void Fill(unsigned long seed);
	private:
		void Init();
		static const int BG_STAR_MAX = 65536;
		Graphics::StaticMesh *m_model;
		Graphics::Shader *m_shader;
	};
	
	class MilkyWay
	{
	public:
		MilkyWay();
		~MilkyWay();
		void Draw(Graphics::Renderer *r);
	private:
		Graphics::StaticMesh *m_model;
	};

	// contains starfield, milkyway, possibly other Background elements
	class Container
	{
	public:
		// default constructor, needs Refresh with proper seed to show starfield
		Container();
		Container(unsigned long seed);
		void Draw(Graphics::Renderer *r, const matrix4x4d &transform) const;
		void Refresh(unsigned long seed);

	private:
		Starfield m_starField;
		MilkyWay m_milkyWay;
	};

}; //namespace Background

#endif
