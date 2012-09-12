// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#ifndef AMBIENTSOUNDS_H
#define AMBIENTSOUNDS_H

class AmbientSounds {
	public:
	static void Init();
	static void Uninit();
	static void Update();
	private:
	static void UpdateForCamType();
};

#endif /* AMBIENTSOUNDS_H */
