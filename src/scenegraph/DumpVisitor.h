// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NodeVisitor.h"

#include <string>
#include <vector>

/*
 * Print the graph structure to console
 * Collect statistics
 */
namespace SceneGraph {

	class Model;

	class DumpVisitor : public NodeVisitor {
	public:
		struct LodStatistics {
			unsigned int nodeCount;
			unsigned int opaqueGeomCount;
			unsigned int transGeomCount;

			unsigned int triangles;
		};

		struct ModelStatistics {
			unsigned int materialCount;
			unsigned int collTriCount;
		};

		DumpVisitor(const Model *m);

		std::string GetModelStatistics();

		virtual void ApplyNode(Node &);
		virtual void ApplyGroup(Group &);
		virtual void ApplyLOD(LOD &);
		virtual void ApplyStaticGeometry(StaticGeometry &);

	private:
		void PutIndent() const;
		void PutNodeName(const Node &) const;

		unsigned int m_level;
		ModelStatistics m_modelStats;
		LodStatistics m_stats;
		std::vector<LodStatistics> m_lodStats;
	};

} // namespace SceneGraph
