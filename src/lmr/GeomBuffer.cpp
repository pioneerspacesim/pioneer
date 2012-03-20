#include "GeomBuffer.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/TextureGL.h" // XXX temporary until LMR uses renderer drawing properly

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

GeomBuffer::GeomBuffer(LmrModel *model, bool isStatic) {
	curOp.type = OP_NONE;
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
	if (curOp.type != OP_NONE) m_ops.push_back(curOp);
	//printf("%d vertices, %d indices, %s\n", m_vertices.size(), m_indices.size(), m_isStatic ? "static" : "dynamic");
	if (m_isStatic && m_indices.size()) {
		s_staticBufferPool.AddGeometry(m_vertices.size(), &m_vertices[0], m_indices.size(), &m_indices[0],
				&m_boIndexBase, &m_bo);
	}
	curOp.type = OP_NONE;
}

void GeomBuffer::FreeGeometry() {
	m_vertices.clear();
	m_indices.clear();
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

void GeomBuffer::Render(Graphics::Renderer *r, const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
	StaticInit(r);

	int activeLights = 0;
	s_numTrisRendered += m_indices.size()/3;
	
	LmrShader *curShader = s_sunlightShader[Graphics::State::GetNumLights()-1].Get();

	BindBuffers();

	glDepthRange(0.0, 1.0);

	const unsigned int opEndIdx = m_ops.size();
	for (unsigned int i=0; i<opEndIdx; i++) {
		const Op &op = m_ops[i];
		switch (op.type) {
		case OP_DRAW_ELEMENTS:
			if (op.elems.textureFile) {
				glEnable(GL_TEXTURE_2D);
				if (!op.elems.texture)
					op.elems.texture = Graphics::TextureBuilder::Model(*op.elems.textureFile).GetOrCreateTexture(r, "model");
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, static_cast<Graphics::TextureGL*>(op.elems.texture)->GetTextureNum());
				if (op.elems.glowmapFile) {
					if (!op.elems.glowmap)
						op.elems.glowmap = Graphics::TextureBuilder::Model(*op.elems.glowmapFile).GetOrCreateTexture(r, "model");
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, static_cast<Graphics::TextureGL*>(op.elems.glowmap)->GetTextureNum());
				}
				UseProgram(curShader, true, op.elems.glowmapFile);
			} else {
				UseProgram(curShader, false);
			}
			if (m_isStatic) {
				// from static VBO
				glDrawElements(GL_TRIANGLES, 
						op.elems.count, GL_UNSIGNED_SHORT,
						BUFFER_OFFSET((op.elems.start+m_boIndexBase)*sizeof(Uint16)));
				//glDrawRangeElements(GL_TRIANGLES, m_boIndexBase + op.elems.elemMin,
				//		m_boIndexBase + op.elems.elemMax, op.elems.count, GL_UNSIGNED_SHORT,
				//		BUFFER_OFFSET((op.elems.start+m_boIndexBase)*sizeof(Uint16)));
			//	glDrawRangeElements(GL_TRIANGLES, op.elems.elemMin, op.elems.elemMax, 
			//		op.elems.count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(op.elems.start*sizeof(Uint16)));
			} else {
				// otherwise regular index vertex array
				glDrawElements(GL_TRIANGLES, op.elems.count, GL_UNSIGNED_SHORT, &m_indices[op.elems.start]);
			}
			if (op.elems.texture) {
				if (op.elems.glowmap) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			}
			break;
		case OP_DRAW_BILLBOARDS: {
			Graphics::UnbindAllBuffers();
			//XXX have to copy positions to a temporary array as
			//renderer::drawpointsprites does not have a stride parameter
			std::vector<vector3f> verts;
			verts.reserve(op.billboards.count);
			for (int j = 0; j < op.billboards.count; j++) {
				verts.push_back(m_vertices[op.billboards.start + j].v);
			}
			if (!op.billboards.texture)
				op.billboards.texture = Graphics::TextureBuilder::Model(*op.billboards.textureFile).GetOrCreateTexture(r, "billboard");
			Graphics::Material mat;
			mat.unlit = true;
			mat.texture0 = op.billboards.texture;
			mat.diffuse = Color(op.billboards.col[0], op.billboards.col[1], op.billboards.col[2], op.billboards.col[3]);
			r->DrawPointSprites(op.billboards.count, &verts[0], &mat, op.billboards.size);
			BindBuffers();
			break;
		}
		case OP_SET_MATERIAL:
			{
				const LmrMaterial &m = m_model->m_materials[op.col.material_idx];
				glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, m.diffuse);
				glMaterialfv (GL_FRONT, GL_SPECULAR, m.specular);
				glMaterialfv (GL_FRONT, GL_EMISSION, m.emissive);
				glMaterialf (GL_FRONT, GL_SHININESS, m.shininess);
				if (m.diffuse[3] >= 1.0) {
					r->SetBlendMode(Graphics::BLEND_SOLID);
				} else {
					r->SetBlendMode(Graphics::BLEND_ALPHA);
				}
			}
			break;
		case OP_ZBIAS:
			if (is_zero_general(op.zbias.amount)) {
				glDepthRange(0.0, 1.0);
			} else {
			//	vector3f tv = cameraPos - vector3f(op.zbias.pos);
			//	if (vector3f::Dot(tv, vector3f(op.zbias.norm)) < 0.0f) {
					glDepthRange(0.0, 1.0 - op.zbias.amount*NEWMODEL_ZBIAS);
			//	} else {
			//		glDepthRange(0.0, 1.0);
			//	}
			}
			break;
		case OP_CALL_MODEL:
			{
			// XXX materials fucked up after this
			const matrix4x4f trans = matrix4x4f(op.callmodel.transform);
			vector3f cam_pos = trans.InverseOf() * cameraPos;
			RenderState rstate2;
			rstate2.subTransform = rstate->subTransform * trans;
			rstate2.combinedScale = rstate->combinedScale * op.callmodel.scale * op.callmodel.model->m_scale;
			op.callmodel.model->Render(r, &rstate2, cam_pos, trans, params);
			// XXX re-binding buffer may not be necessary
			BindBuffers();
			}
			break;
		case OP_LIGHTING_TYPE:
			if (op.lighting_type.local) {
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
			break;
		case OP_USE_LIGHT:
			{
				if (m_model->m_lights.size() <= unsigned(op.light.num)) {
					m_model->m_lights.resize(op.light.num+1);
				}
				LmrLight &l = m_model->m_lights[op.light.num];
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
			break;
		case OP_NONE:
			assert(0);
		}
	}
	
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);

	Graphics::UnbindAllBuffers();

	RenderThrusters(r, rstate, cameraPos, params);

	//XXX hack. Unuse any shader. Can be removed when LMR uses Renderer.
	if (Graphics::AreShadersEnabled())
		s_sunlightShader[0]->Unuse();
}

void GeomBuffer::RenderThrusters(Graphics::Renderer *r, const RenderState *rstate, const vector3f &cameraPos, const LmrObjParams *params) {
	if (m_thrusters.empty()) return;

	glDisable(GL_LIGHTING);
	r->SetBlendMode(Graphics::BLEND_ADDITIVE);
	r->SetDepthWrite(false);
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	for (unsigned int i=0; i<m_thrusters.size(); i++) {
		m_thrusters[i].Render(r, rstate, params);
	}
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.f, 1.f, 1.f);
	r->SetBlendMode(Graphics::BLEND_SOLID);
	r->SetDepthWrite(true);
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
	if (m_putGeomInsideout) {
		m_vertices.push_back(Vertex(pos, -normal, tex_u, tex_v));
	} else {
		m_vertices.push_back(Vertex(pos, normal, tex_u, tex_v));
	}
	return m_vertices.size() - 1;
}

void GeomBuffer::SetVertex(int idx, const vector3f &pos, const vector3f &normal, GLfloat tex_u, GLfloat tex_v) {
	if (m_putGeomInsideout) {
		m_vertices[idx] = Vertex(pos, -normal, tex_u, tex_v);
	} else {
		m_vertices[idx] = Vertex(pos, normal, tex_u, tex_v);
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
	OpDrawElements(3);
	if (m_putGeomInsideout) {
		PushIdx(i1);
		PushIdx(i3);
		PushIdx(i2);
	} else {
		PushIdx(i1);
		PushIdx(i2);
		PushIdx(i3);
	}
	m_triflags.push_back(curTriFlag);
}

void GeomBuffer::PushZBias(float amount, const vector3f &pos, const vector3f &norm) {
	if (curOp.type) m_ops.push_back(curOp);
	curOp.type = OP_ZBIAS;
	curOp.zbias.amount = amount;
	memcpy(curOp.zbias.pos, &pos.x, 3*sizeof(float));
	memcpy(curOp.zbias.norm, &norm.x, 3*sizeof(float));
}

void GeomBuffer::PushSetLocalLighting(bool enable) {
	if (curOp.type) m_ops.push_back(curOp);
	curOp.type = OP_LIGHTING_TYPE;
	curOp.lighting_type.local = enable;
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
	if (curOp.type) m_ops.push_back(curOp);
	curOp.type = OP_USE_LIGHT;
	curOp.light.num = num;
}

void GeomBuffer::PushCallModel(LmrModel *m, const matrix4x4f &transform, float scale) {
	if (curOp.type) m_ops.push_back(curOp);
	curOp.type = OP_CALL_MODEL;
	memcpy(curOp.callmodel.transform, &transform[0], 16*sizeof(float));
	curOp.callmodel.model = m;
	curOp.callmodel.scale = scale;
}

void GeomBuffer::PushInvisibleTri(int i1, int i2, int i3) {
	if (curOp.type != OP_NONE) m_ops.push_back(curOp);
	curOp.type = OP_NONE;
	m_indices.push_back(i1);
	m_indices.push_back(i2);
	m_indices.push_back(i3);
	m_triflags.push_back(curTriFlag);
}

void GeomBuffer::PushBillboards(const char *texname, const float size, const vector3f &color, const int numPoints, const vector3f *points)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "textures/%s", texname);

	if (curOp.type) m_ops.push_back(curOp);
	curOp.type = OP_DRAW_BILLBOARDS;
	curOp.billboards.start = m_vertices.size();
	curOp.billboards.count = numPoints;
	curOp.billboards.textureFile = new std::string(buf);
	curOp.billboards.texture = 0;
	curOp.billboards.col[0] = color.x;
	curOp.billboards.col[1] = color.y;
	curOp.billboards.col[2] = color.z;
	curOp.billboards.col[3] = 1.0f;

	for (int i=0; i<numPoints; i++)
		PushVertex(points[i], vector3f());
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
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_SET_MATERIAL;
		curOp.col.material_idx = (*i).second;
	} else {
		throw LmrUnknownMaterial();
	}
}

int GeomBuffer::AllocVertices(int num) {
	int start = m_vertices.size();
	m_vertices.resize(start + num);
	return start;
}

void GeomBuffer::GetCollMeshGeometry(LmrCollMesh *c, const matrix4x4f &transform, const LmrObjParams *params) {
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
	const unsigned int opEndIdx = m_ops.size();
	for (unsigned int i=0; i<opEndIdx; i++) {
		const Op &op = m_ops[i];
		if (op.type == OP_CALL_MODEL) {
			matrix4x4f _trans = transform * matrix4x4f(op.callmodel.transform);
			op.callmodel.model->GetCollMeshGeometry(c, _trans, params);
		}
	}
}

void GeomBuffer::BindBuffers() {
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
}

void GeomBuffer::OpDrawElements(int numIndices) {
	if ((curOp.type != OP_DRAW_ELEMENTS) || (curOp.elems.textureFile != curTexture)) {
		if (curOp.type) m_ops.push_back(curOp);
		curOp.type = OP_DRAW_ELEMENTS;
		curOp.elems.start = m_indices.size();
		curOp.elems.count = 0;
		curOp.elems.elemMin = 1<<30;
		curOp.elems.elemMax = 0;
		curOp.elems.textureFile = curTexture;
		curOp.elems.texture = 0;
		curOp.elems.glowmapFile = curGlowmap;
		curOp.elems.glowmap = 0;
	}
	curOp.elems.count += numIndices;
}

void GeomBuffer::PushIdx(Uint16 v) {
	curOp.elems.elemMin = std::min<int>(v, curOp.elems.elemMin);
	curOp.elems.elemMax = std::max<int>(v, curOp.elems.elemMax);
	m_indices.push_back(v);
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
