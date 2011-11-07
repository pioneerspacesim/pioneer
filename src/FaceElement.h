#ifndef _FACEELEMENT_H
#define _FACEELEMENT_H

#include "ui/UIManager.h"

// <face gender='male|female' armour='0|1' seed='12345'/>

class FaceElement : public Rocket::Core::Element {
public:
	FaceElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_initted(false), m_tex(0) {}
	virtual ~FaceElement();

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);
	virtual void OnRender();
	virtual void OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes);
	
private:
	bool m_initted;

	GLuint m_tex;

	Uint32 m_seed;
	bool m_armour;
	int m_gender;;
};

#endif
