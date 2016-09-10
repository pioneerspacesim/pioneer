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
#include "OpenGLLibs.h"
#include "../Renderer.h"
#include <stack>
#include <unordered_map>

namespace Graphics {

class Texture;
struct Settings;

namespace GL2 {
	class GasGiantSurfaceMaterial;
	class GeoSphereSkyMaterial;
	class GeoSphereSurfaceMaterial;
	class Material;
	class MultiMaterial;
	class LitMultiMaterial;
	class Program;
	class RenderState;
	class RenderTarget;
	class RingMaterial;
	class FresnelColourMaterial;
	class ShieldMaterial;
}

class RendererGL2 : public Renderer
{
public:
	static void RegisterRenderer();

	RendererGL2(WindowSDL *window, const Graphics::Settings &vs);
	virtual ~RendererGL2();

	virtual const char* GetName() const { return "OpenGL 2.1, with extensions, renderer"; }

	virtual void WriteRendererInfo(std::ostream &out) const;

	virtual void CheckRenderErrors(const char *func = nullptr, const int line = -1) const override final { CheckErrors(func, line); }
	static void CheckErrors(const char *func = nullptr, const int line = -1);

	virtual bool SupportsInstancing() override final { return false; }

	virtual bool GetNearFarRange(float &near, float &far) const;

	virtual bool BeginFrame();
	virtual bool EndFrame();
	virtual bool SwapBuffers();

	virtual bool SetRenderState(RenderState*) override;
	virtual bool SetRenderTarget(RenderTarget*) override;

	virtual bool SetDepthRange(double near_, double far_) override;

	virtual bool ClearScreen();
	virtual bool ClearDepthBuffer();
	virtual bool SetClearColor(const Color &c);

	virtual bool SetViewport(int x, int y, int width, int height);

	virtual bool SetTransform(const matrix4x4d &m);
	virtual bool SetTransform(const matrix4x4f &m);
	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far);
	virtual bool SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
	virtual bool SetProjection(const matrix4x4f &m);

	virtual bool SetWireFrameMode(bool enabled);

	virtual bool SetLights(Uint32 numlights, const Light *l);
	virtual Uint32 GetNumLights() const override final { return m_numLights; }
	virtual bool SetAmbientColor(const Color &c);

	virtual bool SetScissor(bool enabled, const vector2f &pos = vector2f(0.0f), const vector2f &size = vector2f(0.0f));

	virtual bool DrawTriangles(const VertexArray *vertices, RenderState *state, Material *material, PrimitiveType type=TRIANGLES) override;
	virtual bool DrawPointSprites(const Uint32 count, const vector3f *positions, RenderState *rs, Material *material, float size) override;
	virtual bool DrawPointSprites(const Uint32 count, const vector3f *positions, const vector2f *offsets, const float *sizes, RenderState *rs, Material *material) override;
	virtual bool DrawBuffer(VertexBuffer*, RenderState*, Material*, PrimitiveType) override;
	virtual bool DrawBufferIndexed(VertexBuffer*, IndexBuffer*, RenderState*, Material*, PrimitiveType) override;
	virtual bool DrawBufferInstanced(VertexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType type = TRIANGLES) override;
	virtual bool DrawBufferIndexedInstanced(VertexBuffer*, IndexBuffer*, RenderState*, Material*, InstanceBuffer*, PrimitiveType = TRIANGLES) override;

	virtual Material *CreateMaterial(const MaterialDescriptor &descriptor) override;
	virtual Texture *CreateTexture(const TextureDescriptor &descriptor) override;
	virtual RenderState *CreateRenderState(const RenderStateDesc &) override;
	virtual RenderTarget *CreateRenderTarget(const RenderTargetDesc &) override;
	virtual VertexBuffer *CreateVertexBuffer(const VertexBufferDesc&) override;
	virtual IndexBuffer *CreateIndexBuffer(Uint32 size, BufferUsage) override;
	virtual InstanceBuffer *CreateInstanceBuffer(Uint32 size, BufferUsage) override;

	virtual bool ReloadShaders();

	virtual const matrix4x4f& GetCurrentModelView() const { return m_modelViewStack.top(); }
	virtual const matrix4x4f& GetCurrentProjection() const { return m_projectionStack.top(); }
	virtual void GetCurrentViewport(Sint32 *vp) const {
		const Viewport &cur = m_viewportStack.top();
		vp[0] = cur.x; vp[1] = cur.y; vp[2] = cur.w; vp[3] = cur.h;
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
	void EnableVertexAttributes(const VertexBuffer*);
	void EnableVertexAttributes(const VertexArray*);
	//disable previously enabled
	void DisableVertexAttributes(const VertexBuffer*);
	void DisableVertexAttributes();
	int m_numLights;
	int m_numDirLights;
	std::vector<GLuint> m_vertexAttribsSet;
	float m_minZNear;
	float m_maxZFar;
	bool m_useCompressedTextures;

	void SetMaterialShaderTransforms(Material *);

	matrix4x4f& GetCurrentTransform() { return m_currentTransform; }
	matrix4x4f m_currentTransform;

	GL2::Program* GetOrCreateProgram(GL2::Material*);
	friend class GL2::Material;
	friend class GL2::GasGiantSurfaceMaterial;
	friend class GL2::GeoSphereSurfaceMaterial;
	friend class GL2::GeoSphereSkyMaterial;
	friend class GL2::MultiMaterial;
	friend class GL2::LitMultiMaterial;
	friend class GL2::RingMaterial;
	friend class GL2::FresnelColourMaterial;
	friend class GL2::ShieldMaterial;
	std::vector<std::pair<MaterialDescriptor, GL2::Program*> > m_programs;
	std::unordered_map<Uint32, GL2::RenderState*> m_renderStates;
	float m_invLogZfarPlus1;
	GL2::RenderTarget *m_activeRenderTarget;
	RenderState *m_activeRenderState;

	MatrixMode m_matrixMode;
	std::stack<matrix4x4f> m_modelViewStack;
	std::stack<matrix4x4f> m_projectionStack;

	struct Viewport {
		Viewport() : x(0), y(0), w(0), h(0) {}
		Sint32 x, y, w, h;
	};
	std::stack<Viewport> m_viewportStack;

private:
	static bool initted;

	typedef std::map<std::pair<AttributeSet, size_t>, RefCountedPtr<VertexBuffer>> AttribBufferMap;
	typedef AttribBufferMap::iterator AttribBufferIter;
	static AttribBufferMap s_AttribBufferMap;
};

#define RENDERER_CHECK_ERRORS() RendererGL2::CheckErrors(__FUNCTION__, __LINE__)

}

#endif
