#ifndef _TEXTUREMANAGER_H
#define _TEXTUREMANAGER_H

#include "libs.h"
#include <string>

class Texture {
public:
	Texture(const std::string &file_name, bool load_now) {
		this->filename = file_name;
		isLoaded = false;
		tex = 0;
		width = height = -1;
		if (load_now) Load();
	}
	virtual ~Texture() { if (tex) glDeleteTextures(1, &tex); }
	const std::string &GetFilename() const { return filename; }
	bool IsLoaded() const { return isLoaded; }
	void BindTexture() {
		if (!IsLoaded()) Load();
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
private:
	void Load();

	std::string filename;
	GLuint tex;
	int width, height;
	bool isLoaded;
};

namespace TextureManager {
	Texture *GetTexture(const std::string &filename, bool preload = false);
	void Clear();
}

#endif
