#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "libs.h"
#include "Render.h"
/*
 * Classes to draw background stars and the milky way
 * They need to work both using and without using VBOs
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
		Starfield();
		~Starfield();
		void Draw();
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
		void Draw();
	private:
		GLuint m_vbo;
		std::vector<Background::Vertex> m_dataBottom;
		std::vector<Background::Vertex> m_dataTop;
	};

}; //namespace Background

#endif
