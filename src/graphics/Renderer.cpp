// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Renderer.h"
#include "Texture.h"

namespace Graphics {

Renderer::Renderer(WindowSDL *window, int w, int h) :
	m_width(w), m_height(h), m_ambient(Color::BLACK), m_window(window), m_currentModelView(0), m_currentProjection(0), m_matrixMode(GL_MODELVIEW)
{
	for(Uint32 i = 0; i < kMaxStackDepth; i++) {
		m_ModelViewStack[i]		= matrix4x4f::Identity();
		m_ProjectionStack[i]	= matrix4x4f::Identity();
	}
	for(Uint32 i = 0; i < 4; i++) {
		m_currentViewport[i] = 0;
	}
}

Renderer::~Renderer()
{
	RemoveAllCachedTextures();
}

Texture *Renderer::GetCachedTexture(const std::string &type, const std::string &name)
{
	TextureCacheMap::iterator i = m_textures.find(TextureCacheKey(type,name));
	if (i == m_textures.end()) return 0;
	return (*i).second->Get();
}

void Renderer::AddCachedTexture(const std::string &type, const std::string &name, Texture *texture)
{
	RemoveCachedTexture(type,name);
	m_textures.insert(std::make_pair(TextureCacheKey(type,name),new RefCountedPtr<Texture>(texture)));
}

void Renderer::RemoveCachedTexture(const std::string &type, const std::string &name)
{
	TextureCacheMap::iterator i = m_textures.find(TextureCacheKey(type,name));
	if (i == m_textures.end()) return;
	delete (*i).second;
	m_textures.erase(i);
}

void Renderer::RemoveAllCachedTextures()
{
	for (TextureCacheMap::iterator i = m_textures.begin(); i != m_textures.end(); ++i)
		delete (*i).second;
	m_textures.clear();
}

void Renderer::MatrixMode(GLuint mm) 
{ 
	PROFILE_SCOPED()
	glMatrixMode(mm);
	m_matrixMode = mm; 
}

void Renderer::PushMatrix() 
{
	PROFILE_SCOPED()
	
	glPushMatrix();
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		m_ModelViewStack[m_currentModelView+1] = m_ModelViewStack[m_currentModelView];
		++m_currentModelView;	
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:
		m_ModelViewStack[m_currentProjection+1] = m_ModelViewStack[m_currentProjection];
		++m_currentProjection;	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
	assert(m_currentProjection<kMaxStackDepth);
}

void Renderer::PopMatrix() 
{
	PROFILE_SCOPED()
	glPopMatrix();
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		--m_currentModelView;	
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:	
		--m_currentProjection;	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
	assert(m_currentProjection>=0);
}

void Renderer::LoadIdentity()
{
	PROFILE_SCOPED()
	glLoadIdentity();
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		m_ModelViewStack[m_currentModelView] = matrix4x4f::Identity();		
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:
		m_ProjectionStack[m_currentProjection] = matrix4x4f::Identity();	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
}

void Renderer::LoadMatrix(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	glLoadMatrixf(&m[0]);
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		m_ModelViewStack[m_currentModelView] = m;		
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:
		m_ProjectionStack[m_currentProjection] = m;	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
}

void Renderer::Translate( const float x, const float y, const float z ) 
{
	PROFILE_SCOPED()
	glTranslatef(x,y,z);
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		m_ModelViewStack[m_currentModelView].Translate(x,y,z);		
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:
		m_ProjectionStack[m_currentProjection].Translate(x,y,z);	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
}

void Renderer::Scale( const float x, const float y, const float z ) 
{
	PROFILE_SCOPED()
	glScalef(x,y,z);
	switch(m_matrixMode) {
	case GL_MODELVIEW:		
	case GL_MODELVIEW_MATRIX:
		m_ModelViewStack[m_currentModelView].Scale(x,y,z);		
		break;
	case GL_PROJECTION:
	case GL_PROJECTION_MATRIX:
		m_ProjectionStack[m_currentProjection].Scale(x,y,z);	
		break;
	default:
		assert(false && "invalid matrixMode set");
	}
}

}
