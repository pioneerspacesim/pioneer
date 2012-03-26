#include "GeomBuffer.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/TextureGL.h" // XXX temporary until LMR uses renderer drawing properly
#include "graphics/Surface.h"
#include "graphics/VertexArray.h"

#include "LmrModel.h"
#include "Utils.h"

namespace LMR {

BufferObjectPool<sizeof(Vertex)> GeomBuffer::s_staticBufferPool;

bool GeomBuffer::s_initialized = false;
ScopedPtr<GeomBuffer::LmrShader> GeomBuffer::s_sunlightShader[4];
ScopedPtr<GeomBuffer::LmrShader> GeomBuffer::s_pointlightShader[4];

void GeomBuffer::StaticInit(Graphics::Renderer *renderer)
{
	if (s_initialized) return;

	s_sunlightShader[0].Reset(new LmrShader("model", "#define NUM_LIGHTS 1\n"));
	s_sunlightShader[1].Reset(new LmrShader("model", "#define NUM_LIGHTS 2\n"));
	s_sunlightShader[2].Reset(new LmrShader("model", "#define NUM_LIGHTS 3\n"));
	s_sunlightShader[3].Reset(new LmrShader("model", "#define NUM_LIGHTS 4\n"));
	s_pointlightShader[0].Reset(new LmrShader("model-pointlit", "#define NUM_LIGHTS 1\n"));
	s_pointlightShader[1].Reset(new LmrShader("model-pointlit", "#define NUM_LIGHTS 2\n"));
	s_pointlightShader[2].Reset(new LmrShader("model-pointlit", "#define NUM_LIGHTS 3\n"));
	s_pointlightShader[3].Reset(new LmrShader("model-pointlit", "#define NUM_LIGHTS 4\n"));

	ShipThruster::Init(renderer);

	s_initialized = true;
}

GeomBuffer::GeomBuffer(LmrModel *model, bool isStatic, Graphics::Renderer *renderer) :
	m_renderer(renderer),
	m_mesh(new Graphics::Mesh(Graphics::TRIANGLES)),
	m_curMaterialIdx(-1)
{
	curTriFlag = 0;
	curTexture = 0;
	curGlowmap = 0;
	curTexMatrix = matrix4x4f::Identity();
	m_model = model;
	m_isStatic = isStatic;
	m_bo = 0;
	m_putGeomInsideout = false;
}

void GeomBuffer::PreBuild() {
	FreeGeometry();
	curTriFlag = 0;
}

void GeomBuffer::PostBuild() {
	CompleteSurface();

	//printf("%d vertices, %d indices, %s\n", m_vertices.size(), m_indices.size(), m_isStatic ? "static" : "dynamic");
	/*
	if (m_isStatic && m_indices.size()) {
		s_staticBufferPool.AddGeometry(m_vertices.size(), &m_vertices[0], m_indices.size(), &m_indices[0],
				&m_boIndexBase, &m_bo);
	}
	*/
}

void GeomBuffer::FreeGeometry() {
	m_mesh.Reset(new Graphics::Mesh(Graphics::TRIANGLES));
	m_triflags.clear();
	m_ops.clear();
	m_thrusters.clear();
	m_putGeomInsideout = false;
}

int GeomBuffer::s_numTrisRendered;

//binds shader and sets lmr specific uniforms
void GeomBuffer::UseProgram(LmrShader *shader, bool Textured, bool Glowmap) {
	if (Graphics::AreShadersEnabled()) {
		shader->Use();
		if (Textured) shader->set_tex(0);
		shader->set_usetex(Textured ? 1 : 0);
		if (Glowmap) shader->set_texGlow(1);
		shader->set_useglow(Glowmap ? 1 : 0);
	}
}

#define BUFFER_OFFSET(i) (reinterpret_cast<const GLvoid *>(i))
static const float NEWMODEL_ZBIAS = 0.0002f;

void GeomBuffer::Render(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
	StaticInit(m_renderer);

	int activeLights = 0;
	s_numTrisRendered += m_mesh->GetNumIndices()/3;
	
#if 0
	LmrShader *curShader = s_sunlightShader[Graphics::State::GetNumLights()-1].Get();

	BindBuffers();

	glDepthRange(0.0, 1.0);
#endif

	if (m_isStatic) // XXX hack disable dynamic draw
		if (m_mesh->GetNumIndices() > 0)
			m_renderer->DrawMesh(m_mesh.Get());

	for (std::vector<Op*>::const_iterator i = m_ops.begin(); i != m_ops.end(); ++i) {
		switch ((*i)->type) {

		/*
		case OP_DRAW_ELEMENTS: {
			if (op->textureFile) {
				glEnable(GL_TEXTURE_2D);
				if (!op->texture)
					op->texture = Graphics::TextureBuilder::Model(*op->textureFile).GetOrCreateTexture(r, "model");
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, static_cast<Graphics::TextureGL*>(op->texture)->GetTextureNum());
				if (op->glowmapFile) {
					if (!op->glowmap)
						op->glowmap = Graphics::TextureBuilder::Model(*op->glowmapFile).GetOrCreateTexture(r, "model");
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, static_cast<Graphics::TextureGL*>(op->glowmap)->GetTextureNum());
				}
				UseProgram(curShader, true, op->glowmapFile);
			} else {
				UseProgram(curShader, false);
			}
			if (m_isStatic) {
				// from static VBO
				glDrawElements(GL_TRIANGLES, op->count, GL_UNSIGNED_SHORT, BUFFER_OFFSET((op->start+m_boIndexBase)*sizeof(Uint16)));
			} else {
				// otherwise regular index vertex array
				glDrawElements(GL_TRIANGLES, op->count, GL_UNSIGNED_SHORT, &m_indices[op->start]);
			}
			if (op->texture) {
				if (op->glowmap) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			}
			break;
		}
		*/

		case OP_DRAW_BILLBOARDS: {
			OpDrawBillboards *op = static_cast<OpDrawBillboards*>(*i);
			Graphics::UnbindAllBuffers();

			if (!op->texture)
				op->texture = Graphics::TextureBuilder::Model(*op->textureFile).GetOrCreateTexture(m_renderer, "billboard");

			Graphics::Material mat;
			mat.unlit = true;
			mat.texture0 = op->texture;
			mat.diffuse = Color(op->col[0], op->col[1], op->col[2], op->col[3]);

			m_renderer->DrawPointSprites(op->positions.size(), &op->positions[0], &mat, op->size);

			BindBuffers();
			break;
		}

		case OP_ZBIAS: {
#if 0
			OpZBias *op = static_cast<OpZBias*>(*i);
			if (is_zero_general(op->amount)) {
				glDepthRange(0.0, 1.0);
			} else {
			//	vector3f tv = cameraPos - vector3f(op->pos);
			//	if (vector3f::Dot(tv, vector3f(op->norm)) < 0.0f) {
					glDepthRange(0.0, 1.0 - op->amount*NEWMODEL_ZBIAS);
			//	} else {
			//		glDepthRange(0.0, 1.0);
			//	}
			}
#endif
			break;
		}

		case OP_CALL_MODEL:
			{
			OpCallModel *op = static_cast<OpCallModel*>(*i);
			// XXX materials fucked up after this
			const matrix4x4f trans = matrix4x4f(op->transform);
			vector3f cam_pos = trans.InverseOf() * cameraPos;
			RenderState rstate2;
			rstate2.subTransform = rstate->subTransform * trans;
			rstate2.combinedScale = rstate->combinedScale * op->scale * op->model->m_scale;
			op->model->Render(&rstate2, cam_pos, trans, params);
			// XXX re-binding buffer may not be necessary
			BindBuffers();
			}
			break;

		case OP_LIGHTING_TYPE: {
#if 0
			OpLightingType *op = static_cast<OpLightingType*>(*i);
			if (op->local) {
				glDisable(GL_LIGHT0);
				glDisable(GL_LIGHT1);
				glDisable(GL_LIGHT2);
				glDisable(GL_LIGHT3);
				float zilch[4] = { 0.0f,0.0f,0.0f,0.0f };
				for (int j=4; j<8; j++) {
					// so why are these set each
					// time? because the shader
					// path does not know if
					// lightsources are active and
					// uses them all (4-8)
					glLightfv(GL_LIGHT0+j, GL_DIFFUSE, zilch);
					glLightfv(GL_LIGHT0+j, GL_SPECULAR, zilch);
				}
				activeLights = 0;
			} else {
				int numLights = Graphics::State::GetNumLights();
				for (int j=0; j<numLights; j++) glEnable(GL_LIGHT0 + j);
				for (int j=4; j<8; j++) glDisable(GL_LIGHT0 + j);
				curShader = s_sunlightShader[Graphics::State::GetNumLights()-1].Get();
			}
#endif
			break;
		}

		case OP_USE_LIGHT:
#if 0
			{
				OpUseLight *op = static_cast<OpUseLight*>(*i);
				if (m_model->m_lights.size() <= unsigned(op->num)) {
					m_model->m_lights.resize(op->num+1);
				}
				LmrLight &l = m_model->m_lights[op->num];
				glEnable(GL_LIGHT0 + 4 + activeLights);
				glLightf(GL_LIGHT0 + 4 + activeLights, GL_QUADRATIC_ATTENUATION, l.quadraticAttenuation);
				glLightfv(GL_LIGHT0 + 4 + activeLights, GL_POSITION, l.position);
				glLightfv(GL_LIGHT0 + 4 + activeLights, GL_DIFFUSE, l.color);
				glLightfv(GL_LIGHT0 + 4 + activeLights, GL_SPECULAR, l.color);
				curShader = s_pointlightShader[activeLights++].Get();
				if (activeLights > 4) {
					Error("Too many active lights in model '%s' (maximum 4)", m_model->GetName());
				}
			}
#endif
			break;

		default:
			assert(0);
		}
	}
	
#if 0
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);

	Graphics::UnbindAllBuffers();
#endif

	RenderThrusters(rstate, cameraPos, params);

#if 0
	//XXX hack. Unuse any shader. Can be removed when LMR uses Renderer.
	if (Graphics::AreShadersEnabled())
		s_sunlightShader[0]->Unuse();
#endif
}

void GeomBuffer::RenderThrusters(const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
	if (m_thrusters.empty()) return;

	glDisable(GL_LIGHTING);
	m_renderer->SetBlendMode(Graphics::BLEND_ADDITIVE);
	m_renderer->SetDepthWrite(false);
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	for (unsigned int i=0; i<m_thrusters.size(); i++) {
		m_thrusters[i].Render(m_renderer, rstate, params);
	}
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.f, 1.f, 1.f);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	m_renderer->SetDepthWrite(true);
	glEnable(GL_CULL_FACE);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
}

void GeomBuffer::PushThruster(const vector3f &pos, const vector3f &dir, const float power, bool linear_only) {
	unsigned int i = m_thrusters.size();
	m_thrusters.resize(i+1);
	m_thrusters[i].m_pos = pos;
	m_thrusters[i].m_dir = dir;
	m_thrusters[i].m_power = power;
	m_thrusters[i].m_linear_only = linear_only;
}

int GeomBuffer::PushVertex(const vector3f &pos, const vector3f &normal) {
	vector3f tex = curTexMatrix * pos;
	return PushVertex(pos, normal, tex.x, tex.y);
}

void GeomBuffer::SetVertex(int idx, const vector3f &pos, const vector3f &normal) {
	vector3f tex = curTexMatrix * pos;
	SetVertex(idx, pos, normal, tex.x, tex.y);
}

int GeomBuffer::PushVertex(const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v) {
	EnsureSurface();
	Graphics::VertexArray *va = m_curSurface->GetVertices();
	if (m_putGeomInsideout) {
		va->Add(pos, -normal, vector2f(tex_u, tex_v));
	} else {
		va->Add(pos, normal, vector2f(tex_u, tex_v));
	}
	return va->GetNumVerts()-1;
}

void GeomBuffer::SetVertex(int idx, const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v) {
	EnsureSurface();
	// XXX still not sure if VertexArray should be indexable or if we should
	//     only do vertex pushes. in the mean time, we reach into VertexArray
	//     and handle it ourselves
	Graphics::VertexArray *va = m_curSurface->GetVertices();
	if (m_putGeomInsideout) {
		va->position[idx] = pos;
		va->normal[idx] = -normal;
		va->uv0[idx] = vector2f(tex_u,tex_v);
	} else {
		va->position[idx] = pos;
		va->normal[idx] = normal;
		va->uv0[idx] = vector2f(tex_u,tex_v);
	}
}

void GeomBuffer::SetTexture(const char *tex) {
	if (tex) {
		curTexture = new std::string(tex);
	} else {
		curTexture = 0;
		curGlowmap = 0; //won't have these without textures
	}
}

void GeomBuffer::SetGlowMap(const char *tex) {
	if (tex) {
		curGlowmap = new std::string(tex);
	} else {
		curGlowmap = 0;
	}
}

void GeomBuffer::PushTri(int i1, int i2, int i3) {
	EnsureSurface();
	std::vector<Uint16> &indices = m_curSurface->GetIndices();
	if (m_putGeomInsideout) {
		indices.push_back(i1);
		indices.push_back(i3);
		indices.push_back(i2);
	} else {
		indices.push_back(i1);
		indices.push_back(i2);
		indices.push_back(i3);
	}
	m_triflags.push_back(curTriFlag);
}

void GeomBuffer::PushZBias(float amount) {
    PushOp(new OpZBias(amount));
}

void GeomBuffer::PushSetLocalLighting(bool enable) {
	PushOp(new OpLightingType(enable));
}

void GeomBuffer::SetLight(int num, float quadratic_attenuation, const vector3f &pos, const vector3f &col) {
	if (m_model->m_lights.size() <= unsigned(num)) {
		m_model->m_lights.resize(num+1);
	}
	LmrLight &l = m_model->m_lights[num];
	memcpy(l.position, &pos, sizeof(vector3f));
	memcpy(l.color, &col, sizeof(vector3f));
	l.position[3] = l.color[3] = 1.0;
	l.quadraticAttenuation = quadratic_attenuation;
}

void GeomBuffer::PushUseLight(int num) {
	PushOp(new OpUseLight(num));
}

void GeomBuffer::PushCallModel(LmrModel *m, const matrix4x4f &transform, float scale) {
	PushOp(new OpCallModel(m, transform, scale));
}

void GeomBuffer::PushInvisibleTri(int i1, int i2, int i3) {
	/* XXX dunno
	m_indices.push_back(i1);
	m_indices.push_back(i2);
	m_indices.push_back(i3);
	m_triflags.push_back(curTriFlag);
	*/
}

void GeomBuffer::PushBillboards(const char *texname, const float size, const Color &color, const int numPoints, const vector3f *points)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "textures/%s", texname);

	OpDrawBillboards *op = new OpDrawBillboards;
	op->textureFile = new std::string(buf);
	op->texture = 0;
	op->size = size;
	op->col = color;

	for (int i=0; i<numPoints; i++)
		op->positions.push_back(points[i]);	// XXX how about just dropping *points in?

	PushOp(op);
}

void GeomBuffer::SetMaterial(const char *mat_name, const float mat[11]) {
	std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
	if (i != m_model->m_materialLookup.end()) {
		LmrMaterial &m = m_model->m_materials[(*i).second];
		m.diffuse[0] = mat[0];
		m.diffuse[1] = mat[1];
		m.diffuse[2] = mat[2];
		m.diffuse[3] = mat[3];
		m.specular[0] = mat[4];
		m.specular[1] = mat[5];
		m.specular[2] = mat[6];
		m.specular[3] = 1.0f;
		m.shininess = Clamp(mat[7], 1.0f, 100.0f);
		m.emissive[0] = mat[8];
		m.emissive[1] = mat[9];
		m.emissive[2] = mat[10];
		m.emissive[3] = 1.0f;
	} else {
		fprintf(stderr, "Unknown material name '%s'", mat_name);
		abort(); // XXX use fallback
	}
}

void GeomBuffer::PushUseMaterial(const char *mat_name) {
	std::map<std::string, int>::iterator i = m_model->m_materialLookup.find(mat_name);
	if (i != m_model->m_materialLookup.end()) {
		m_curMaterialIdx = (*i).second;
		CompleteSurface();
	} else {
		throw LmrUnknownMaterial();
	}
}

int GeomBuffer::AllocVertices(int num) {
	// XXX grow VertexArray lists manually for now
	EnsureSurface();
	Graphics::VertexArray *va = m_curSurface->GetVertices();
	int start = va->GetNumVerts();
	va->position.resize(start+num);
	va->normal.resize(start+num);
	va->uv0.resize(start+num);
	return start;
}

void GeomBuffer::GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform, const LmrObjParams *params) {
#if 0
	const int vtxBase = c->nv;
	const int idxBase = c->ni;
	const int flagBase = c->nf;
	c->nv += m_vertices.size();
	c->ni += m_indices.size();
	c->nf += m_indices.size()/3;
	assert(m_triflags.size() == m_indices.size()/3);
	c->m_numTris += m_triflags.size();

	if (m_vertices.size()) {
		c->pVertex = static_cast<float*>(realloc(c->pVertex, 3*sizeof(float)*c->nv));
	
		for (unsigned int i=0; i<m_vertices.size(); i++) {
			const vector3f v = transform * m_vertices[i].v;
			c->pVertex[3*vtxBase + 3*i] = v.x;
			c->pVertex[3*vtxBase + 3*i+1] = v.y;
			c->pVertex[3*vtxBase + 3*i+2] = v.z;
			c->m_aabb.Update(vector3d(v));
		}
	}
	if (m_indices.size()) {
		c->pIndex = static_cast<int*>(realloc(c->pIndex, sizeof(int)*c->ni));
		c->pFlag = static_cast<unsigned int*>(realloc(c->pFlag, sizeof(unsigned int)*c->nf));
		for (unsigned int i=0; i<m_indices.size(); i++) {
			c->pIndex[idxBase + i] = vtxBase + m_indices[i];
		}
		for (unsigned int i=0; i<m_triflags.size(); i++) {
			c->pFlag[flagBase + i] = m_triflags[i];
		}
	}
	
	// go through Ops to see if we call other models
	for (std::vector<Op*>::const_iterator i = m_ops.begin(); i != m_ops.end(); ++i) {
		if ((*i)->type == OP_CALL_MODEL) {
			OpCallModel *op = static_cast<OpCallModel*>(*i);
			matrix4x4f _trans = transform * matrix4x4f(op->transform);
			op->model->GetCollMeshGeometry(c, _trans, params);
		}
	}
#endif
}

void GeomBuffer::BindBuffers() {
#if 0
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);

	if (m_isStatic) {
		if (m_bo) m_bo->BindBuffersForDraw();
	} else {
		Graphics::UnbindAllBuffers();
		if (m_vertices.size()) {
			glNormalPointer(GL_FLOAT, sizeof(Vertex), &m_vertices[0].n);
			glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &m_vertices[0].v);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m_vertices[0].tex_u);
		}
	}
#endif
}

void GeomBuffer::CompleteSurface()
{
	if (m_curSurface && m_curSurface->GetNumIndices() > 0)
		m_mesh->AddSurface(m_curSurface.Release());
}

void GeomBuffer::EnsureSurface()
{
	// XXX better to check the existing surface and take it if it has the same
	// material, otherwise complete it and move on
	if (m_curSurface) return;

	// XXX prep renderer material from material/texture/shader/lights
	Graphics::Material *mat = new Graphics::Material;
	if (m_curMaterialIdx >= 0) {
		const LmrMaterial &lmrMat = m_model->m_materials[m_curMaterialIdx];

		mat->diffuse = lmrMat.diffuse;
		// XXX no renderer support for these yet
		//mat->specular = lmrMat.specular;
		//mat->emissive = lmrMat.emissive;
		//mat->shininess = lmrMat.shininess
		
		if (curTexture)
			mat->texture0 = Graphics::TextureBuilder::Model(*curTexture).GetOrCreateTexture(m_renderer);

		mat->unlit = true;
	}
		
	m_curSurface.Reset(new Graphics::Surface(Graphics::TRIANGLES, new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0), RefCountedPtr<Graphics::Material>(mat)));
}

void GeomBuffer::PushOp(Op *op)
{
	m_ops.push_back(op);

	// XXX op probably changed materials or wants to call model, so current
	// surface is finished. add it to the mesh
	CompleteSurface();
}

void GeomBuffer::SaveToCache(FILE *f) {
#if 0
	int numVertices = m_vertices.size();
	int numIndices = m_indices.size();
	int numTriflags = m_triflags.size();
	int numThrusters = m_thrusters.size();
	int numOps = m_ops.size();
	fwrite(&numVertices, sizeof(numVertices), 1, f);
	fwrite(&numIndices, sizeof(numIndices), 1, f);
	fwrite(&numTriflags, sizeof(numTriflags), 1, f);
	fwrite(&numThrusters, sizeof(numThrusters), 1, f);
	fwrite(&numOps, sizeof(numOps), 1, f);
	if (numVertices) fwrite(&m_vertices[0], sizeof(Vertex), numVertices, f);
	if (numIndices) fwrite(&m_indices[0], sizeof(Uint16), numIndices, f);
	if (numTriflags) fwrite(&m_triflags[0], sizeof(Uint16), numTriflags, f);
	if (numThrusters) fwrite(&m_thrusters[0], sizeof(ShipThruster::Thruster), numThrusters, f);
	if (numOps) {
		for (int i=0; i<numOps; i++) {
			fwrite(&m_ops[i], sizeof(Op), 1, f);
			if (m_ops[i].type == OP_CALL_MODEL) {
				_fwrite_string(m_ops[i].callmodel.model->GetName(), f);
			}
			else if ((m_ops[i].type == OP_DRAW_ELEMENTS) && (m_ops[i].elems.texture)) {
				_fwrite_string(m_ops[i].elems.texture->GetFilename(), f);
				if (m_ops[i].elems.glowmap)
					_fwrite_string(m_ops[i].elems.glowmap->GetFilename(), f);
			}
			else if ((m_ops[i].type == OP_DRAW_BILLBOARDS) && (m_ops[i].billboards.texture)) {
				_fwrite_string(m_ops[i].billboards.texture->GetFilename(), f);
			}
		}
	}
#endif
}
void GeomBuffer::LoadFromCache(FILE *f) {
#if 0
	int numVertices, numIndices, numTriflags, numThrusters, numOps;
	fread_or_die(&numVertices, sizeof(numVertices), 1, f);
	fread_or_die(&numIndices, sizeof(numIndices), 1, f);
	fread_or_die(&numTriflags, sizeof(numTriflags), 1, f);
	fread_or_die(&numThrusters, sizeof(numThrusters), 1, f);
	fread_or_die(&numOps, sizeof(numOps), 1, f);
	assert(numVertices <= 65536);
	assert(numIndices < 1000000);
	assert(numTriflags < 1000000);
	assert(numThrusters < 1000);
	assert(numOps < 1000);
	if (numVertices) {
		m_vertices.resize(numVertices);
		fread_or_die(&m_vertices[0], sizeof(Vertex), numVertices, f);
	}
	if (numIndices) {
		m_indices.resize(numIndices);
		fread_or_die(&m_indices[0], sizeof(Uint16), numIndices, f);
	}
	if (numTriflags) {
		m_triflags.resize(numTriflags);
		fread_or_die(&m_triflags[0], sizeof(Uint16), numTriflags, f);
	}
	if (numThrusters) {
		m_thrusters.resize(numThrusters);
		fread_or_die(&m_thrusters[0], sizeof(ShipThruster::Thruster), numThrusters, f);
	}
	m_ops.resize(numOps);
	for (int i=0; i<numOps; i++) {
		fread_or_die(&m_ops[i], sizeof(Op), 1, f);
		if (m_ops[i].type == OP_CALL_MODEL) {
			m_ops[i].callmodel.model = s_models[_fread_string(f)];
		}
		else if ((m_ops[i].type == OP_DRAW_ELEMENTS) && (m_ops[i].elems.texture)) {
			m_ops[i].elems.texture = s_textureCache->GetModelTexture(_fread_string(f));
			m_ops[i].elems.texture = 0;

			if (m_ops[i].elems.glowmap) {
				m_ops[i].elems.glowmap = s_textureCache->GetModelTexture(_fread_string(f));
				m_ops[i].elems.glowmap = 0;
			}
		}
		else if ((m_ops[i].type == OP_DRAW_BILLBOARDS) && (m_ops[i].billboards.texture)) {
			m_ops[i].billboards.texture = s_textureCache->GetBillboardTexture(_fread_string(f));
			m_ops[i].elems.texture = 0;
		}
	}
#endif
}

}
