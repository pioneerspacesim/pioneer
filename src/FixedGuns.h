#ifndef FIXEDGUNS_H
#define FIXEDGUNS_H

#include "vector3.h"
#include "libs.h"
#include "Space.h"
#include "Camera.h"
#include "json/JsonUtils.h"
#include "scenegraph/Model.h"

class FixedGuns
{
	public:
		FixedGuns();
		~FixedGuns();
		void InitGun( SceneGraph::Model *m, const char *tag, int num);
	protected:
	private:
		enum {
			GUN_FRONT,
			GUN_REAR,
			GUNMOUNT_MAX = 2
		};

		struct Gun {
			vector3f pos;
			vector3f dir;
			Uint32 state;
			float recharge;
			float temperature;
		};
		//TODO: Make it a vector?
		Gun m_gun[2];
};

#endif // FIXEDGUNS_H
