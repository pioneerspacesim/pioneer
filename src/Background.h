#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "render/Render.h"
/*
 * Classes to draw background stars and the milky way
 */

namespace Background
{
	#pragma pack(4)
	struct Vertex
	{
		Vertex() :
			x(0.f), y(0.f), z(0.f),
			r(0.f), g(0.f), b(0.f)
		{ }

		Vertex(const float& _x, const float& _y, const float& _z) :
			x(_x), y(_y), z(_z),
			r(1.f), g(1.f), b(1.f)
		{ }

		//we really need a Color class
		Vertex(const vector3f v, const vector3f c) :
			x(v.x),	y(v.y),	z(v.z),
			r(c.x), g(c.y), b(c.z)
		{ }
		float x, y, z;
		float r, g, b;
	};
	#pragma pack()

	class Starfield
	{
	public:
		//does not Fill the starfield
		Starfield();
		Starfield(unsigned long seed);
		~Starfield();
		void Draw() const;
		//create or recreate the starfield
		void Fill(unsigned long seed);
	private:
		static const int BG_STAR_MAX = 65536;
		GLuint m_vbo;
		Render::Shader *m_shader;
		Background::Vertex m_stars[BG_STAR_MAX];
	};
	
	class MilkyWay
	{
	public:
		MilkyWay();
		~MilkyWay();
		void Draw() const;
	private:
		GLuint m_vbo;
		std::vector<Background::Vertex>::size_type m_bottomSize;
		std::vector<Background::Vertex>::size_type m_topSize;
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
