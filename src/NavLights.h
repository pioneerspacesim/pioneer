// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _NAVLIGHTS_H
#define _NAVLIGHTS_H
/*
 * Blinking navigation lights for ships and stations
 */
#include "libs.h"
#include "Serializer.h"
#include "json/json.h"

namespace Graphics { class Renderer; }
namespace SceneGraph { class Model; class Billboard; }

class NavLights
{
public:
	enum LightColor {
		NAVLIGHT_RED    = 0,
		NAVLIGHT_GREEN  = 1,
		NAVLIGHT_BLUE   = 2,
		NAVLIGHT_YELLOW = 3
	};

	struct LightBulb
	{
		LightBulb(Uint8 group, Uint8 mask, Uint8 color, SceneGraph::Billboard *bb);
		Uint8 group;
		Uint8 mask; //bitmask: 00001111 light on half the period, 11111111 light on the entire period etc...
		Uint8 color;
		SceneGraph::Billboard *billboard;
	};

	NavLights(SceneGraph::Model*, float period = 2.f);
	virtual ~NavLights();
	virtual void SaveToJson(Json::Value &jsonObj);
	virtual void LoadFromJson(const Json::Value &jsonObj);

	void SetEnabled(bool on) { m_enabled = on; }
	void Update(float time);
	void SetColor(unsigned int group, LightColor);

	static void Init(Graphics::Renderer*);
	static void Uninit();

protected:

	class TGroupLights {
	public:
		TGroupLights(Uint32 g) : m_group(g) {}
		const Uint32 m_group;
		std::vector<LightBulb> m_lights;
	private:
		TGroupLights() : m_group(0xFFFFFFFF) {}
	};

	// for use with std::find_if
	class GroupMatch{
		const Uint32 group;
	public:
		GroupMatch(const Uint32 g): group(g) {}
		bool operator() (const TGroupLights& myValue)
		{ 
			return (group == myValue.m_group);
		}
	};

	typedef std::vector<TGroupLights> GroupLightsVec;
	typedef GroupLightsVec::iterator GroupLightsVecIter;
	GroupLightsVec m_groupLights;
	float m_time;
	float m_period;
	bool m_enabled;
};

#endif
