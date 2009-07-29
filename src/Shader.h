#ifndef _SHADER_H
#define _SHADER_H

namespace Shader {
	enum VertexProgram {
		VPROG_GEOSPHERE, VPROG_SBRE, VPROG_SIMPLE, VPROG_MAX
	};
	void ToggleState();
	void EnableVertexProgram(VertexProgram p);
	void DisableVertexProgram();
	void Init();
}

#endif /* _SHADER_H */
