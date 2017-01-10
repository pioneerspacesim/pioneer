#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "vector3.h"
#include "libs.h"
#include "Space.h"
#include "Camera.h"
#include "json/JsonUtils.h"
#include "scenegraph/Model.h"
#include "Projectile.h"

class FixedGuns
{
	public:
		FixedGuns();
		~FixedGuns();
		void Init();
		void InitGun( SceneGraph::Model *m, const char *tag, int num);
		void UpdateGuns( float timeStep );
		bool IsFiring();
		inline float GetGunTemperature(int idx) const { return m_gun[idx].temperature; }
		inline void IsDual( int idx, bool dual ) { m_gun[idx].dual = dual; };
		/* TODO: Define a Struct here (or in Projectile) that every object
		 * that would fire something must share: you then could pass it to
		 * DefineGun and Finally to Projectile ( AFAIK there's nothing that
		 * can call Projectile class until now...)
		*/
		void DefineGun( int num, float recharge, float lifespan, float dam, float length, float width, bool mining, const Color& color, float speed );

		void Fire( int num, Body* b, const matrix3x3d& shipOrient, const vector3d& shipVel, const vector3d& shipPos );
		inline void SetCoolingBoost( float cooler ) { m_cooler_boost = cooler; };
		inline void SetState( int idx, float state ) { m_gun[idx].state = state; };
		bool IsGunReady( int num );
	protected:
		virtual void SaveToJson(int i, Json::Value &jsonObj );
		virtual void LoadFromJson(int i, const Json::Value &jsonObj );
	private:
		enum {
			GUN_FRONT,
			GUN_REAR,
			GUNMOUNT_MAX = 2
		};

		struct ProjectileData {
			float lifespan;
			float damage;
			float length;
			float width;
			bool mining;
			float speed;
			Color color;
		};

		struct Gun {
			vector3f pos;
			vector3f dir;
			Uint32 state;
			float recharge;
			float temperature;
			bool dual;
			// Better if the one below is a pointer...
			ProjectileData projData;
		};
		//TODO: Make it a vector and rework struct Gun to have bool dir={Forward,Backward}
		Gun m_gun[GUNMOUNT_MAX];
		float m_cooler_boost;
};

#endif // FIXEDGUNS_H
