#ifndef _SHADER_H
#define _SHADER_H

namespace Shader {
	enum VertexProgram {
		VPROG_GEOSPHERE, VPROG_SBRE, VPROG_SIMPLE, VPROG_POINTSPRITE, VPROG_PLANETHORIZON, VPROG_MAX
	};
	bool IsEnabled();
	bool IsVtxProgActive();
	GLint GetActiveProgram();
	void ToggleState();
	void EnableVertexProgram(VertexProgram p);
	void DisableVertexProgram();
	void Init();
}

#endif /* _SHADER_H */
