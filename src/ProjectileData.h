#ifndef PROJECTILEDATA_H_INCLUDED
#define PROJECTILEDATA_H_INCLUDED

#include "Color.h"
#include "Json.h"
#include "GameSaveError.h"
#include "utils.h"

struct ProjectileData {
	ProjectileData() :
		lifespan(0.0f),
		damage(0.0f),
		length(0.0f),
		width(0.0f),
		speed(0.0f),
		color(Color::WHITE),
		mining(false),
		beam(false) {}
	ProjectileData(float ls, float d, float l, float w, float s, Color c, bool m, bool b) :
		lifespan(ls),
		damage(d),
		length(l),
		width(w),
		speed(s),
		color(c),
		mining(m),
		beam(b) {}
	ProjectileData(const Json &jsonObj)
	{
		try {
		lifespan = jsonObj["life_span"];
		damage = jsonObj["base_dam"];
		length = jsonObj["length"];
		width = jsonObj["width"];
		speed = jsonObj["speed"];
		color = jsonObj["color"];
		mining = jsonObj["mining"];
		beam = jsonObj["is_beam"];
		} catch (Json::type_error &) {
			Output("Got error loading '%s'\n", __func__);
			throw SavedGameCorruptException();
		}
	}
	Json SaveToJson()
	{
		Json jsonObj;
		jsonObj["life_span"] = lifespan;
		jsonObj["base_dam"] = damage;
		jsonObj["length"] = length;
		jsonObj["width"] = width;
		jsonObj["speed"] = speed;
		jsonObj["color"] = color;
		jsonObj["mining"] = mining;
		jsonObj["is_beam"] = beam;
		return jsonObj;
	}
	float lifespan;
	float damage;
	float length;
	float width;
	float speed;
	Color color;
	bool mining;
	bool beam;
};


#endif // PROJECTILEDATA_H_INCLUDED
