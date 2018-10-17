// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "vector3.h"
#include "libs.h"
#include "Space.h"
#include "Camera.h"
#include "JsonUtils.h"
#include "scenegraph/Model.h"
#include "Projectile.h"
#include "DynamicBody.h"

enum Guns {
	GUN_FRONT,
	GUN_REAR,
	GUNMOUNT_MAX = 2
};

class FixedGuns : public RefCounted
{
	public:
		FixedGuns();
		virtual ~FixedGuns();
		void Init(DynamicBody *b);
		void InitGuns( SceneGraph::Model *m);
		void UpdateGuns( float timeStep );
		bool Fire( const int num, Body* b );
		bool IsFiring();
		bool IsFiring(const int num);
		bool IsBeam(const int num);
		float GetGunTemperature(int idx) const;
		inline void IsDual( int idx, bool dual ) { m_gun[idx].dual = dual; };
		void MountGun( const int num, const float recharge, const float lifespan, const float damage, const float length,
						 const float width, const bool mining, const Color& color, const float speed, const bool beam, const float heatrate, const float coolrate);
		void UnMountGun( int num );
		inline float GetGunRange( int idx ) { return m_gun[idx].projData.speed*m_gun[idx].projData.lifespan; };
		inline float GetProjSpeed(int idx ) { return m_gun[idx].projData.speed; };
		inline void SetCoolingBoost( float cooler ) { m_cooler_boost = cooler; };
		inline void SetGunFiringState( int idx, int s ) {
			if(m_gun_present[idx])
				m_is_firing[idx] = s;
		};
		virtual void SaveToJson( Json &jsonObj, Space *space);
		virtual void LoadFromJson( const Json &jsonObj, Space *space);
	private:

		struct GunData {
			struct GunLoc {
				vector3d pos;
				vector3d dir;
			};
			std::vector<GunLoc> locs;
			float recharge;
			float temp_heat_rate;
			float temp_cool_rate;
			bool dual;
			ProjectileData projData;
		};

		bool m_is_firing[Guns::GUNMOUNT_MAX];
		float m_recharge_stat[Guns::GUNMOUNT_MAX];
		float m_temperature_stat[Guns::GUNMOUNT_MAX];
		//TODO: Make it a vector and rework struct Gun to have bool dir={Forward,Backward}
		bool m_gun_present[Guns::GUNMOUNT_MAX];
		GunData m_gun[Guns::GUNMOUNT_MAX];
		float m_cooler_boost;
};

#endif // FIXEDGUNS_H
