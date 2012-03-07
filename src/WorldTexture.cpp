#include "WorldTexture.h"

WorldTextureDescriptor::WorldTextureDescriptor(const std::string &_filename) :
	Graphics::TextureDescriptor(TYPE_WORLD, TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_UNSIGNED_BYTE), Options(Options::REPEAT, Options::LINEAR, true)),
	filename(_filename)
{
}

const Graphics::TextureDescriptor::Data *WorldTextureDescriptor::GetData() const 
{
	return GetDataFromFile(filename);
}
