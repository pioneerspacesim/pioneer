#include "GuiTexture.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"

using namespace Graphics;

namespace Gui {

SurfaceTextureDescriptor::SurfaceTextureDescriptor(const std::string &_name, SDL_Surface *surface) :
    Graphics::TextureDescriptor(TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_UNSIGNED_BYTE), Options(Options::CLAMP, Options::LINEAR, false)),
	name(_name), m_surface(surface)
{
}

const Graphics::TextureDescriptor::Data *SurfaceTextureDescriptor::GetData() const {
	assert(m_surface);
	return GetDataFromSurface(m_surface, true);
}


FileTextureDescriptor::FileTextureDescriptor(const std::string &_filename) :
    Graphics::TextureDescriptor(TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_UNSIGNED_BYTE), Options(Options::CLAMP, Options::LINEAR, false)),
	filename(_filename)
{
}

const Graphics::TextureDescriptor::Data *FileTextureDescriptor::GetData() const {
	return GetDataFromFile(filename, true);
}


void TexturedQuad::Draw(Graphics::Renderer *renderer, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint)
{
    Graphics::VertexArray va(ATTRIB_POSITION | ATTRIB_UV0);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(texPos.x,           texPos.y));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(texPos.x+texSize.x, texPos.y));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));

	Graphics::Material m;
	m.unlit = true;
	m.texture0 = m_texture;
	m.vertexColors = false;
	m.diffuse = tint;
	renderer->DrawTriangles(&va, &m, TRIANGLE_STRIP);
}

}
