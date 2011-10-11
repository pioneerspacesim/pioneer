#include "libs.h"

class Texture {
public:
	Texture(const std::string &file_name, bool load_now) {
		this->filename = file_name;
		isLoaded = false;
		tex = 0;
		if (load_now) Load();
	}
	virtual ~Texture() { if (tex) glDeleteTextures(1, &tex); }
	const std::string &GetFilename() const { return filename; }
	bool IsLoaded() const { return isLoaded; }
	void BindTexture() {
		if (!IsLoaded()) Load();
		glBindTexture(GL_TEXTURE_2D, tex);
	}
private:
	void Load();

	std::string filename;
	GLuint tex;
	bool isLoaded;
};

namespace TextureManager {
	extern Texture *GetTexture(const char *filename, bool preload = false);
	static inline Texture *GetTexture(const std::string &filename, bool preload = false) { return GetTexture(filename.c_str(), preload); }
	extern void Clear();
}

