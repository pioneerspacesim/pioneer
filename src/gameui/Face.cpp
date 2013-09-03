// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Face.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "graphics/TextureBuilder.h"
#include "FaceGenManager.h"

using namespace UI;

namespace GameUI {

static const Uint32 FACE_WIDTH = 295;
static const Uint32 FACE_HEIGHT = 285;

RefCountedPtr<Graphics::Material> Face::s_material;

static void _blit_image(const SDLSurfacePtr &s, SDLSurfacePtr &is, int xoff, int yoff)
{
	// XXX what should this do if the image couldn't be loaded?
	if (! is) { return; }

	SDL_Rect destrec = { 0, 0, 0, 0 };
	destrec.x = ((FACE_WIDTH-is->w-1)/2)+xoff;
	destrec.y = yoff;
	SDL_BlitSurface(is.Get(), 0, s.Get(), &destrec);
}

Face::Face(Context *context, Uint32 flags, Uint32 seed) : Single(context)
{
	if (!seed) seed = time(0);
	Random rand(seed);

	m_flags = flags;
	m_seed = seed;

	int race = rand.Int32(0,2);

	int gender;
	switch (flags & GENDER_MASK) {
		case MALE:
			gender = 0;
			break;
		case FEMALE:
			gender = 1;
			break;
		case RAND:
		default:
			gender = rand.Int32(0,1);
			break;
	}

	const int head  = rand.Int32(1,FaceGenManager::NumHeads(race))-1;
	const int eyes  = rand.Int32(1,FaceGenManager::NumEyes(race))-1;
	const int nose  = rand.Int32(1,FaceGenManager::NumNoses(race))-1;
	const int mouth = rand.Int32(1,FaceGenManager::NumMouths(race))-1;
	const int hair  = rand.Int32(1,FaceGenManager::NumHairstyles(race))-1;

	const int clothes		= rand.Int32(1,FaceGenManager::NumClothes())-1;
	const int armour		= rand.Int32(1,FaceGenManager::NumArmour())-1;
	const int accessories	= rand.Int32(1,FaceGenManager::NumAccessories())-1;
	const int background	= rand.Int32(1,FaceGenManager::NumBackground())-1;
	
	FaceGenManager::TQueryResult res;
	FaceGenManager::GetImagesForCharacter(res, race, gender, head, eyes, 
		nose, mouth, hair, clothes, armour, accessories, background);

	SDLSurfacePtr faceim = SDLSurfacePtr::WrapNew(SDL_CreateRGBSurface(SDL_SWSURFACE, FACE_WIDTH, FACE_HEIGHT, 24, 0xff, 0xff00, 0xff0000, 0));

	_blit_image(faceim, res.mBackground, 0, 0);
	_blit_image(faceim, res.mHead, 0, 0);

	if (!(flags & ARMOUR)) {
		_blit_image(faceim, res.mClothes, 0, 135);
	}

	_blit_image(faceim, res.mEyes, 0, 41);
	_blit_image(faceim, res.mNose, 1, 89);
	_blit_image(faceim, res.mMouth, 0, 155);

	if (!(flags & ARMOUR)) {
		if (rand.Int32(0,1)>0)
			_blit_image(faceim, res.mAccessories, 0, 0);

		_blit_image(faceim, res.mHairstyle, 0, 0);
	}
	else {
		_blit_image(faceim, res.mArmour, 0, 0);
	}

	m_texture.Reset(Graphics::TextureBuilder(faceim, Graphics::LINEAR_CLAMP, true, true).CreateTexture(GetContext()->GetRenderer()));

	if (!s_material) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		s_material.Reset(GetContext()->GetRenderer()->CreateMaterial(matDesc));
	}
}

void Face::Layout()
{
	Point size(GetSize());
	Point activeArea(std::min(size.x, size.y));
	Point activeOffset(std::max(0, (size.x-activeArea.x)/2), std::max(0, (size.y-activeArea.y)/2));
	SetActiveArea(activeArea, activeOffset);

	Widget *innerWidget = GetInnerWidget();
	if (!innerWidget) return;
	SetWidgetDimensions(innerWidget, activeOffset, activeArea);
	innerWidget->Layout();
}

void Face::Draw()
{
	const Point &offset = GetActiveOffset();
	const Point &area = GetActiveArea();

	const float x = offset.x;
	const float y = offset.y;
	const float sx = area.x;
	const float sy = area.y;

	const vector2f texSize = m_texture->GetDescriptor().texSize;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(x,    y,    0.0f), vector2f(0.0f,      0.0f));
	va.Add(vector3f(x,    y+sy, 0.0f), vector2f(0.0f,      texSize.y));
	va.Add(vector3f(x+sx, y,    0.0f), vector2f(texSize.x, 0.0f));
	va.Add(vector3f(x+sx, y+sy, 0.0f), vector2f(texSize.x, texSize.y));

	Graphics::Renderer *r = GetContext()->GetRenderer();
	s_material->texture0 = m_texture.Get();
	r->DrawTriangles(&va, s_material.Get(), Graphics::TRIANGLE_STRIP);

	Single::Draw();
}

}
