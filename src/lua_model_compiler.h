#ifndef _LUA_MODEL_COMPILER_H
#define _LUA_MODEL_COMPILER_H

void LuaModelCompilerInit();
void LuaModelRender(const char *name, const matrix4x4f &transform);
int LuaModelGetStatsTris();
void LuaModelClearStatsTris();

#endif /* _LUA_MODEL_COMPILER_H */
