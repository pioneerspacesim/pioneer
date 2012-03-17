#ifndef _LMRCOMPILER_H
#define _LMRCOMPILER_H

#include <vector>

class LmrModel;

namespace LMR {
	extern void ModelCompilerInit();
	extern void ModelCompilerUninit();
	extern LmrModel *LookupModelByName(const char *name);
	extern void GetModelsWithTag(const char *tag, std::vector<LmrModel*> &outModels);
}

#endif
