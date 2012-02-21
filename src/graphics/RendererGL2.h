#include "Renderer.h"
#include "RendererLegacy.h"

namespace Graphics {

/*
 * OpenGL 2.x renderer
 *  - no fixed function support (shaders for everything)
 *  The plan is: make this more like GL3/ES2
 *  - try to stick to bufferobjects
 *  - use glvertexattribpointer instead of glvertexpointer etc
 */
class RendererGL2 : public RendererLegacy //XXX shares enough with legacy renderer now
{
public:
	RendererGL2(int width, int height);
	virtual ~RendererGL2();

	virtual const char* GetName() { return "GL2 renderer"; }

	virtual bool BeginFrame();

	virtual bool SetPerspectiveProjection(float fov, float aspect, float near, float far);

	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color *colors, LineType type=LINE_SINGLE);
	virtual bool DrawLines(int vertCount, const vector3f *vertices, const Color &color, LineType type=LINE_SINGLE);

protected:
	virtual void ApplyMaterial(const Material *mat);
	virtual void UnApplyMaterial(const Material *mat);
};

}
