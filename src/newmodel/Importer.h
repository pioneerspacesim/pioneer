#ifndef _IMPORTER_H
#define _IMPORTER_H
/*
 * Model loading works through this (using assimp etc.)
 */
namespace Graphics { class Renderer; }
namespace Newmodel {

class NModel;

class Importer
{
public:
	//create a model with one static geometry node
	NModel *CreateDummyModel(Graphics::Renderer *r);
};

}
#endif