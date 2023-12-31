// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DumpVisitor.h"
#include "Group.h"
#include "LOD.h"
#include "Model.h"
#include "Node.h"
#include "StaticGeometry.h"
#include "utils.h"
#include <iostream>
#include <sstream>

namespace SceneGraph {
	DumpVisitor::DumpVisitor(const Model *m) :
		m_level(0),
		m_stats()
	{
		//model statistics that cannot be visited)
		m_modelStats.collTriCount = m->GetCollisionMesh() ? m->GetCollisionMesh()->GetNumTriangles() : 0;
		m_modelStats.materialCount = m->GetNumMaterials();
	}

	std::string DumpVisitor::GetModelStatistics()
	{
		std::ostringstream ss;

		// Print collected statistics per lod
		if (m_lodStats.empty())
			m_lodStats.push_back(m_stats);

		std::vector<LodStatistics>::iterator it = m_lodStats.begin();
		unsigned int idx = 1;
		while (it != m_lodStats.end()) {
			ss << "\nLOD " << idx << '\n';
			ss << "Nodes: " << it->nodeCount << '\n';
			ss << "Geoms: " << it->opaqueGeomCount << " opaque, " << it->transGeomCount << " transparent\n";
			ss << "Triangles: " << it->triangles << '\n';
			++it;
			idx++;
		};

		ss << '\n';
		ss << "Materials: " << m_modelStats.materialCount << '\n';
		ss << "Collision triangles: " << m_modelStats.collTriCount << '\n';

		return ss.str();
	}

	void DumpVisitor::ApplyNode(Node &n)
	{
		PutNodeName(n);

		m_stats.nodeCount++;
	}

	void DumpVisitor::ApplyGroup(Group &g)
	{
		PutNodeName(g);

		m_level++;
		g.Traverse(*this);
		m_level--;

		m_stats.nodeCount++;
	}

	void DumpVisitor::ApplyLOD(LOD &l)
	{
		ApplyNode(l);

		m_level++;
		for (unsigned int i = 0; i < l.GetNumChildren(); i++) {
			l.GetChildAt(i)->Accept(*this);
			m_lodStats.push_back(m_stats);
			memset(&m_stats, 0, sizeof(LodStatistics));
		}
		m_level--;
	}

	void DumpVisitor::ApplyStaticGeometry(StaticGeometry &g)
	{
		if (g.GetNodeMask() & NODE_TRANSPARENT)
			m_stats.transGeomCount++;
		else
			m_stats.opaqueGeomCount++;

		for (unsigned int i = 0; i < g.GetNumMeshes(); i++)
			m_stats.triangles += g.GetMeshAt(i).indexBuffer->GetSize() / 3;

		ApplyNode(static_cast<Node &>(g));
	}

	void DumpVisitor::PutNodeName(const Node &g) const
	{
		if (g.GetName().empty())
			Log::Info("{0:{1}}{2}", " ", m_level * 2, g.GetTypeName());
		else
			Log::Info("{0:{1}}{2} - {3}", " ", m_level * 2, g.GetTypeName(), g.GetName());
	}
} // namespace SceneGraph
