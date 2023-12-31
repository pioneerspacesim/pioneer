// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
#include "Billboard.h"
#include "CollisionGeometry.h"
#include "FileSystem.h"
#include "LOD.h"
#include "StaticGeometry.h"
#include "Thruster.h"
#include <functional>

namespace Serializer {
	class Reader;
	class Writer;
} // namespace Serializer

namespace SceneGraph {
	class Label3D;
	class Model;

	// Attempt at version history:
	// 1:	prototype
	// 2:	converted StaticMesh to VertexBuffer
	// 3:	store processed collision mesh
	// 4:	compressed SGM files and instancing support
	// 5:	normal mapping
	// 6:	32-bit indicies
	// 6.1:	rewrote serialization, use lz4 compression instead of INFLATE/DEFLATE. Still compatible.
	// 6.2: ignored StaticGeometry::m_blendMode in files. Still write blank value.
	// 7:   Added discrete Tag node, tags are registered in the model hierarchy instead of at the root.
	constexpr Uint32 SGM_VERSION = 7;

	class BinaryConverter : public BaseLoader {
	public:
		BinaryConverter(Graphics::Renderer *);
		void Save(const std::string &filename, Model *m);
		void Save(const std::string &filename, const std::string &savepath, Model *m, const bool bInPlace);
		Model *Load(const std::string &filename);
		Model *Load(const std::string &filename, const std::string &path);
		Model *Load(const std::string &filename, RefCountedPtr<FileSystem::FileData> binfile);

		//if you implement any new node types, you must also register a loader function
		//before calling Load.
		void RegisterLoader(const std::string &typeName, std::function<Node *(NodeDatabase &)>);

	private:
		Model *CreateModel(const std::string &filename, Serializer::Reader &);
		void SaveMaterials(Serializer::Writer &, Model *m);
		void LoadMaterials(Serializer::Reader &);
		void SaveAnimations(Serializer::Writer &, Model *m);
		void LoadAnimations(Serializer::Reader &);
		ModelDefinition FindModelDefinition(const std::string &);

		Node *LoadNode(Serializer::Reader &);
		//this is a very simple loader so it's implemented here
		static Label3D *LoadLabel3D(NodeDatabase &);

		bool m_patternsUsed;
		std::map<std::string, std::function<Node *(NodeDatabase &)>> m_loaders;
	};
} // namespace SceneGraph

#endif
