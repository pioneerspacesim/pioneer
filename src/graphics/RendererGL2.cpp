#include "Shader.h"
#include "Material.h"
#include "Graphics.h"
#include "RendererGL2.h"
#include "RendererGLBuffers.h"
#include "Texture.h"
#include "TextureGL.h"
#include "VertexArray.h"
#include "gl2/Material.h"
#include "gl2/MultiMaterial.h"
#include "gl2/Program.h"

namespace Graphics {

typedef std::vector<std::pair<MaterialDescriptor, GL2::Program*> >::const_iterator ProgramIterator;

// for material-less line and point drawing
GL2::Program *vtxColorProg;
GL2::Program *flatColorProg;

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{
	//the range is very large due to a "logarithmic z-buffer" trick used
	//http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html
	//http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix/
	m_minZNear = 0.0001f;
	m_maxZFar = 10000000.0f;

	MaterialDescriptor desc;
	desc.vertexColors = true;
	vtxColorProg = GetOrCreateProgram(desc);
	flatColorProg = GetOrCreateProgram(MaterialDescriptor());
}

RendererGL2::~RendererGL2()
{
	while (!m_programs.empty()) delete m_programs.back().second, m_programs.pop_back();
}

bool RendererGL2::BeginFrame()
{
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool RendererGL2::SetPerspectiveProjection(float fov, float aspect, float near, float far)
{
	double ymax = near * tan(fov * M_PI / 360.0);
	double ymin = -ymax;
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(xmin, xmax, ymin, ymax, near, far);

	// update values for log-z hack
	Graphics::State::SetZnearZfar(near, far);
	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color *c, LineType t)
{
	if (count < 2 || !v) return false;

	vtxColorProg->Use();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glColorPointer(4, GL_FLOAT, sizeof(Color), c);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	vtxColorProg->Unuse();

	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	flatColorProg->Use();
	flatColorProg->diffuse.Set(c);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	flatColorProg->Unuse();

	return true;
}

Material *RendererGL2::CreateMaterial(const MaterialDescriptor &desc)
{
	GL2::Program *p = 0;
	try {
		p = GetOrCreateProgram(desc);
	} catch (GL2::ShaderException &) {
		// in release builds, the game does not quit instantly but attempts to revert
		// to a 'shaderless' state
		return RendererLegacy::CreateMaterial(desc);
	}

	// Create the material
	GL2::Material *mat = new GL2::MultiMaterial();
	mat->m_program = p;
	mat->newStyleHack = true;
	return mat;
}

void RendererGL2::ApplyMaterial(const Material *mat)
{
	assert(mat && mat->newStyleHack);
	const_cast<Material*>(mat)->Apply();
}

void RendererGL2::UnApplyMaterial(const Material *mat)
{
	const_cast<Material*>(mat)->Unapply();
}

GL2::Program *RendererGL2::GetOrCreateProgram(const MaterialDescriptor &desc)
{
	// search cache first
	GL2::Program *p = 0;
	for (ProgramIterator it = m_programs.begin(); it != m_programs.end(); ++it) {
		if ((*it).first == desc) {
			p = (*it).second;
			break;
		}
	}

	// Pick & create a new program
	if (!p) {
		p = new GL2::MultiProgram(desc);
		m_programs.push_back(std::make_pair(desc, p));
	}

	assert(p != 0);
	return p;
}

}
