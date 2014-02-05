// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_BINARYCONVERTER_H
#define _SCENEGRAPH_BINARYCONVERTER_H

/**
 * Saving and loading a model from a binary format,
 * completely without Assimp
 * Nodes are expected to implement a Save method to
 * serialize their internals
 */

#include "BaseLoader.h"
#include "LOD.h"
#include "StaticGeometry.h"
#include "CollisionGeometry.h"
#include "Thruster.h"
#include "Billboard.h"
#include <functional>

namespace SceneGraph
{
class BinaryConverter : public BaseLoader
{
public:
	BinaryConverter(Graphics::Renderer*);
	void Save(const std::string& filename, Model* m);
	Model *Load(const std::string &filename);
	Model *Load(const std::string &filename, const std::string &path);

	//if you implement any new node types, you must also register a loader function
	//before calling Load.
	void RegisterLoader(const std::string &typeName, std::function<Node*(NodeDatabase&)>);

private:
	Model *CreateModel(Serializer::Reader&);
	void SaveMaterials(Serializer::Writer&, Model* m);
	void LoadMaterials(Serializer::Reader&);
	void SaveAnimations(Serializer::Writer&, Model* m);
	void LoadAnimations(Serializer::Reader&);
	ModelDefinition FindModelDefinition(const std::string&);

	Node* LoadNode(Serializer::Reader&);
	void LoadChildren(Serializer::Reader&, Group* parent);
	//this is a very simple loader so it's implemented here
	static Label3D *LoadLabel3D(NodeDatabase&);

	bool m_patternsUsed;
	std::map<std::string, std::function<Node*(NodeDatabase&)> > m_loaders;
};
}

#endif
