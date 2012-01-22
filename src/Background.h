#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "render/Render.h"

class BufferThing;
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
		void Draw();
		//create or recreate the starfield
		void Fill(unsigned long seed);
	private:
		static const int BG_STAR_MAX = 65536;
		BufferThing *m_model;
		VertexArray *m_stars;
		Render::Shader *m_shader;
	};
	
	class MilkyWay
	{
	public:
		MilkyWay();
		~MilkyWay();
		void Draw();
	private:
		BufferThing *m_model;
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
