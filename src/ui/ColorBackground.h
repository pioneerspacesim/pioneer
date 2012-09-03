#ifndef _UI_COLORBACKGROUND_H
#define _UI_COLORBACKGROUND_H

#include "Single.h"
#include "Color.h"
#include "graphics/Material.h"

namespace UI {

class ColorBackground : public Single {
public:
	virtual void Draw();

	void SetColor(const Color &color) { m_material->diffuse = color; }

protected:
	friend class Context;
	ColorBackground(Context *context, const Color &color);

private:
    ScopedPtr<Graphics::Material> m_material;
};

}

#endif
