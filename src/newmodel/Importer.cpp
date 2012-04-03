#include "Importer.h"
#include "Newmodel.h"
#include "StaticGeometry.h"
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Renderer.h"

namespace Newmodel {

NModel *Importer::CreateDummyModel(Graphics::Renderer *renderer)
{
	const std::string modelname("data/models/ships/natrix/lod2.obj");
	const std::string texname("models/ships/natrix/hull.png");

	Assimp::Importer importer;
	//only triangular meshes (and no points, lines either)
	const aiScene *scene = importer.ReadFile(modelname, aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenSmoothNormals);
	if (!scene)
		throw importer.GetErrorString();
	aiMesh *mesh = scene->mMeshes[0];

	Graphics::VertexArray *vts =
		new Graphics::VertexArray(
			Graphics::ATTRIB_POSITION |
			Graphics::ATTRIB_NORMAL |
			Graphics::ATTRIB_UV0);
	RefCountedPtr<Graphics::Material> mat(new Graphics::Material());
	mat->texture0 = Graphics::TextureBuilder::Model(texname).GetOrCreateTexture(renderer, "model");
	mat->diffuse = Color(1.f, 1.f, 1.f, 1.f);
	mat->unlit = false;
	Graphics::Surface *surface = new Graphics::Surface(Graphics::TRIANGLES, vts, mat);
	std::vector<unsigned short> &indices = surface->GetIndices();

	//copy indices first
	for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
		const aiFace *face = &mesh->mFaces[f];
		for (unsigned int i = 0; i < face->mNumIndices; i++) {
			indices.push_back(face->mIndices[i]);
		}
	}

	//then vertices, making gross assumptions of the format
	for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
		const aiVector3D &vtx = mesh->mVertices[v];
		const aiVector3D &norm = mesh->mNormals[v];
		const aiVector3D &uv0 = mesh->mTextureCoords[0][v];
		vts->Add(vector3f(vtx.x, vtx.y, vtx.z),
			vector3f(norm.x, norm.y, norm.z),
			vector2f(uv0.x, 1.f - uv0.y));
	}
	NModel *m = new NModel();
	StaticGeometry* geom = new StaticGeometry();
	Graphics::StaticMesh *smesh = geom->GetMesh();
	smesh->AddSurface(surface);
	m->m_root->AddChild(geom);
	return m;
}

}