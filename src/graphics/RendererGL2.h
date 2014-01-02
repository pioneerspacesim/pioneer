// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RENDERER_GL2_H
#define _RENDERER_GL2_H
/*
 * OpenGL 2.X renderer (2.0, GLSL 1.10 at the moment)
 *  - no fixed function support (shaders for everything)
 *  The plan is: make this more like GL3/ES2
 *  - try to stick to bufferobjects
 *  - use glvertexattribpointer instead of glvertexpointer etc
 *  - get rid of built-in glMaterial, glMatrix use
 */
#include "Renderer.h"
#include <stack>

namespace Graphics {

class Texture;
struct Settings;

namespace GL2 {
	class GeoSphereSkyMaterial;
	class GeoSphereSurfaceMaterial;
	class Material;
	class MultiMaterial;
	class LitMultiMaterial;
	class Program;
	class RenderTarget;
	class RingMaterial;
	class FresnelColourMaterial;
}

class RendererGL2 : public Renderer
{
public:
	RendererGL2(WindowSDL *window, const Graphics::Settings &vs);
	virtual ~RendererGL2();

	virtual const char* GetName() const { return "GL2 renderer"; }
	virtual bool GetNearFarRange(float &near, float &far) const;

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool SetRenderTarget(RenderTarget*);

	virtual bool ClearScreen();
	virtual bool ClearDepthBuffer();
	virtual bool SetClearColor(const Color &c);

	virtual bool SetViewport(int x, int y, int width, int height);

	virtual bool SetTransform(const matrix4x4d &m);
	virtual bool SetTransform(const matrix4x4f &m);
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far);
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
	virtual bool SetProjection(const matrix4x4f &m);

	virtual bool SetBlendMode(BlendMode mode);
	virtual bool SetDepthTest(bool enabled);
	virtual bool SetDepthWrite(bool enabled);
	virtual bool SetWireFrameMode(bool enabled);

	virtual bool SetLightsEnabled(const bool enabled);
	virtual bool SetLights(int numlights, const Light *l);
	virtual bool SetAmbientColor(const Color &c);

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f));

	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, LineType type=LINE_SINGLE);
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, LineType type=LINE_SINGLE);
	virtual bool DrawLines2D(int vertCount, const vector2f *vertices, const Color &color, LineType type=LINE_SINGLE);
	virtual bool DrawPoints(int count, const vector3f *points, const Color *colors, float pointSize=1.f);
	virtual bool DrawTriangles(const VertexArray *vertices, Material *material, PrimitiveType type=TRIANGLES);
	virtual bool DrawSurface(const Surface *surface);
	virtual bool DrawPointSprites(int count, const vector3f *positions, Material *material, float size);
	virtual bool DrawStaticMesh(StaticMesh *thing);

	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor);
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor);
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &);

	virtual bool ReloadShaders();

	virtual bool PrintDebugInfo(std::ostream &out);

	virtual const matrix4x4f& GetCurrentModelView() const { return m_modelViewStack.top(); }
	virtual const matrix4x4f& GetCurrentProjection() const { return m_projectionStack.top(); }
	virtual void GetCurrentViewport(Sint32 *vp) const {
		for(int i=0; i<4; i++)
			vp[i] = m_currentViewport[i];
	}

	virtual void SetMatrixMode(MatrixMode mm);
	virtual void PushMatrix();
	virtual void PopMatrix();
	virtual void LoadIdentity();
	virtual void LoadMatrix(const matrix4x4f &m);
	virtual void Translate( const float x, const float y, const float z );
	virtual void Scale( const float x, const float y, const float z );

protected:
	virtual void PushState();
	virtual void PopState();

	//figure out states from a vertex array and enable them
	//also sets vertex pointers
	virtual void EnableClientStates(const VertexArray *v);
	//disable previously enabled
	virtual void DisableClientStates();
	int m_numLights;
	int m_numDirLights;
	std::vector<GLenum> m_clientStates;
	virtual bool BufferStaticMesh(StaticMesh *m);
	float m_minZNear;
	float m_maxZFar;
	bool m_useCompressedTextures;

	matrix4x4f& GetCurrentTransform() { return m_currentTransform; }
	matrix4x4f m_currentTransform;

	GL2::Program* GetOrCreateProgram(GL2::Material*);
	friend class GL2::GeoSphereSurfaceMaterial;
	friend class GL2::GeoSphereSkyMaterial;
	friend class GL2::MultiMaterial;
	friend class GL2::LitMultiMaterial;
	friend class GL2::RingMaterial;
	friend class GL2::FresnelColourMaterial;
	std::vector<std::pair<MaterialDescriptor, GL2::Program*> > m_programs;
	float m_invLogZfarPlus1;
	GL2::RenderTarget *m_activeRenderTarget;

	std::stack<matrix4x4f> m_modelViewStack;
	std::stack<matrix4x4f> m_projectionStack;
	Sint32 m_currentViewport[4];
	MatrixMode m_matrixMode;
};

}

#endif
