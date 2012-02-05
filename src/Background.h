#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "render/Render.h"

class StaticMesh;
struct Material;
struct VertexArray;
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
		void Draw(Renderer *r);
		//create or recreate the starfield
		void Fill(unsigned long seed);
	private:
		void Init();
		static const int BG_STAR_MAX = 65536;
		StaticMesh *m_model;
		VertexArray *m_stars;
		Render::Shader *m_shader;
		Material *m_material;
	};
	
	class MilkyWay
	{
	public:
		MilkyWay();
		~MilkyWay();
		void Draw(Renderer *r);
	private:
		StaticMesh *m_model;
	};

	// contains starfield, milkyway, possibly other Background elements
	class Container
	{
	public:
		// default constructor, needs Refresh with proper seed to show starfield
		Container();
		Container(unsigned long seed);
		void Draw(const matrix4x4d &transform) const;
		void Refresh(unsigned long seed);

	private:
		Starfield m_starField;
		MilkyWay m_milkyWay;
	};

}; //namespace Background

#endif
