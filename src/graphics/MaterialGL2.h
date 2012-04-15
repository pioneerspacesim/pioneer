#ifndef _MATERIALGL2_H
#define _MATERIALGL2_H

#include "Material.h"

namespace Graphics {

class MaterialGL2 : public Material
{
public:
	MaterialGL2() : Material() { }
	~MaterialGL2() { }
	void Apply() const;
	void Unapply() const;
};

}

#endif