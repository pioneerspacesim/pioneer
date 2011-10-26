#include "RocketGaugeElement.h"
#include "RocketGaugeTypeBar.h"

RocketGaugeElement::RocketGaugeElement(const Rocket::Core::String &tag) :
	Rocket::Core::Element(tag)
{
	//default will be a horizontal bar, fill direction right
	type = 0;
	type = new RocketGaugeTypeBar(this);
	type_name = "bar";
	SetClass(type_name, true);
}

RocketGaugeElement::~RocketGaugeElement()
{
	delete type;
}

Rocket::Core::String RocketGaugeElement::GetName() const
{
	return GetAttribute<Rocket::Core::String>("name", "");
}

void RocketGaugeElement::SetName(const Rocket::Core::String& name)
{
	SetAttribute("name", name);
}

Rocket::Core::String RocketGaugeElement::GetValue() const
{
	return Rocket::Core::String(32, "%f", m_value);
}

void RocketGaugeElement::SetValue(const Rocket::Core::String& value)
{
	SetAttribute("value", value);
	//OnAttributeChange should fire after this I believe
}

void RocketGaugeElement::OnUpdate()
{
	type->OnUpdate();
}

void RocketGaugeElement::OnRender()
{
	type->OnRender();
}

void RocketGaugeElement::OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes)
{
	if (changed_attributes.find("type") != changed_attributes.end())
	{
		Rocket::Core::String new_type_name = GetAttribute< Rocket::Core::String >("type", "bar");
		if (new_type_name != type_name)
		{
			RocketGaugeType* new_type = 0;
			
			if (new_type_name == "bar")
				new_type = new RocketGaugeTypeBar(this);

			if (new_type != 0)
			{
				//delete old
				delete type;
				type = new_type;
				SetClass(type_name, false);
				SetClass(new_type_name, true);
				type_name = new_type_name;

				DirtyLayout();
			}
		}
	}

	if (changed_attributes.find("value") != changed_attributes.end()) {
		m_value = GetAttribute< float >("value", 0.f);
	}
	//type should probably do something now!

	if (!type->OnAttributeChange(changed_attributes))
		DirtyLayout();
}

bool RocketGaugeElement::GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions)
{
	return false;
}

class RocketGaugeElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new RocketGaugeElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void RocketGaugeElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new RocketGaugeElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("gauge", instancer);
	instancer->RemoveReference();
}