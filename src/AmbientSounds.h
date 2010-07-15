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
