// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CommandBufferGL.h"
#include "MaterialGL.h"
#include "Program.h"
#include "RenderStateCache.h"
#include "RendererGL.h"
#include "Shader.h"
#include "TextureGL.h"
#include "UniformBuffer.h"
#include "VertexBufferGL.h"

#include "graphics/VertexBuffer.h"

using namespace Graphics::OGL;

void CommandList::AddDrawCmd(Graphics::MeshObject *mesh, Graphics::Material *material, Graphics::InstanceBuffer *inst, uint32_t offset, uint32_t num)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");
	OGL::Material *mat = static_cast<OGL::Material *>(material);

	DrawCmd cmd;
	cmd.mesh = static_cast<OGL::MeshObject *>(mesh);
	cmd.inst = static_cast<OGL::InstanceBuffer *>(inst);
	cmd.offset = offset;
	if (num)
		cmd.numElems = num;
	else
		cmd.numElems = mesh->GetIndexBuffer() ? mesh->GetIndexBuffer()->GetIndexCount() : mesh->GetVertexBuffer()->GetSize();

	cmd.program = mat->EvaluateVariant();
	cmd.shader = mat->GetShader();
	cmd.renderStateHash = mat->m_renderStateHash;
	cmd.drawData = SetupMaterialData(mat);

	m_drawCmds.emplace_back(std::move(cmd));
}

// RenderPass commands can be combined as long as they're setting the same render target
void CommandList::AddRenderPassCmd(RenderTarget *renderTarget, ViewportExtents extents)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	RenderPassCmd *lastCmd = IsEmpty() ? nullptr : std::get_if<RenderPassCmd>(&m_drawCmds.back());
	if (!lastCmd || !lastCmd->setRenderTarget || lastCmd->renderTarget != renderTarget) {
		RenderPassCmd cmd;
		cmd.renderTarget = renderTarget;
		cmd.extents = extents;
		cmd.setRenderTarget = true;
		cmd.clearColors = false;
		cmd.clearDepth = false;

		m_drawCmds.emplace_back(std::move(cmd));
	} else {
		lastCmd->renderTarget = renderTarget;
		lastCmd->extents = extents;
		lastCmd->setRenderTarget = true;
	}
}

// Clear commands can be combined with a previous clear/set render pass command
void CommandList::AddClearCmd(bool clearColors, bool clearDepth, Color color)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	RenderPassCmd *lastCmd = IsEmpty() ? nullptr : std::get_if<RenderPassCmd>(&m_drawCmds.back());

	if (!lastCmd) {
		RenderPassCmd cmd;
		cmd.setRenderTarget = false;
		cmd.clearColors = clearColors;
		cmd.clearDepth = clearDepth;
		cmd.clearColor = color;

		m_drawCmds.emplace_back(std::move(cmd));
	} else {
		lastCmd->clearDepth |= clearDepth;
		if (clearColors) {
			lastCmd->clearColors = true;
			lastCmd->clearColor = color;
		}
	}
}

void CommandList::Reset()
{
	assert(!m_executing && "Attempt to reset a command list while it's being executed!");

	for (auto &bucket : m_dataBuckets)
		bucket.used = 0;

	m_drawCmds.clear();
}

template <size_t I>
size_t align(size_t t)
{
	return (t + (I - 1)) & ~(I - 1);
}

char *CommandList::AllocDrawData(const Shader *shader)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	size_t bufferSize = align<8>(shader->GetNumBufferBindings() * sizeof(UniformBufferBinding));
	size_t textureSize = align<8>(shader->GetNumTextureBindings() * sizeof(Texture *));
	size_t totalSize = constantSize + bufferSize + textureSize;

	char *alloc = nullptr;
	for (auto &bucket : m_dataBuckets) {
		if ((alloc = bucket.alloc(totalSize)))
			break;
	}

	if (!alloc) {
		DataBucket bucket{};
		bucket.data.reset(new char[BUCKET_SIZE]);
		alloc = bucket.alloc(totalSize);
		m_dataBuckets.emplace_back(std::move(bucket));
	}

	assert(alloc != nullptr);
	memset(alloc, '\0', totalSize);
	return alloc;
}

UniformBufferBinding *CommandList::getBufferBindings(const Shader *shader, char *data)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	return reinterpret_cast<UniformBufferBinding *>(data + constantSize);
}

TextureGL **CommandList::getTextureBindings(const Shader *shader, char *data)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	size_t bufferSize = align<8>(shader->GetNumBufferBindings() * sizeof(UniformBufferBinding));
	return reinterpret_cast<TextureGL **>(data + constantSize + bufferSize);
}

char *CommandList::SetupMaterialData(OGL::Material *mat)
{
	PROFILE_SCOPED()
	mat->UpdateDrawData();
	const Shader *s = mat->GetShader();

	char *alloc = AllocDrawData(s);
	memcpy(alloc, mat->m_pushConstants.get(), s->GetConstantStorageSize());

	UniformBufferBinding *buffers = getBufferBindings(s, alloc);
	for (size_t index = 0; index < s->GetNumBufferBindings(); index++) {
		auto &info = mat->m_bufferBindings[index];
		buffers[index].buffer = info.buffer.Get();
		buffers[index].offset = info.offset;
		buffers[index].size = info.size;
	}

	TextureGL **textures = getTextureBindings(s, alloc);
	for (size_t index = 0; index < s->GetNumTextureBindings(); index++) {
		textures[index] = static_cast<TextureGL *>(mat->m_textureBindings[index]);
	}

	return alloc;
}

void CommandList::ApplyDrawData(const DrawCmd &cmd) const
{
	PROFILE_SCOPED();
	RenderStateCache *state = m_renderer->GetStateCache();
	state->SetProgram(cmd.program);

	UniformBufferBinding *buffers = getBufferBindings(cmd.shader, cmd.drawData);
	for (auto &info : cmd.shader->GetBufferBindings())
		state->SetBufferBinding(info.binding, buffers[info.index]);

	TextureGL **textures = getTextureBindings(cmd.shader, cmd.drawData);
	for (auto &info : cmd.shader->GetTextureBindings())
		state->SetTexture(info.binding, textures[info.index]);

	for (auto &info : cmd.shader->GetPushConstantBindings()) {
		GLuint location = cmd.program->GetConstantLocation(info.index);
		if (location == GL_INVALID_INDEX)
			continue;

		Uniform setter(location);
		switch (info.format) {
		case ConstantDataFormat::DATA_FORMAT_INT:
			setter.Set(*reinterpret_cast<int *>(cmd.drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT:
			setter.Set(*reinterpret_cast<float *>(cmd.drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT3:
			setter.Set(*reinterpret_cast<vector3f *>(cmd.drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT4:
			setter.Set(*reinterpret_cast<Color4f *>(cmd.drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_MAT3:
			setter.Set(*reinterpret_cast<matrix3x3f *>(cmd.drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_MAT4:
			setter.Set(*reinterpret_cast<matrix4x4f *>(cmd.drawData + info.offset));
			break;
		default:
			assert(false);
			break;
		}
	}
}

void CommandList::CleanupDrawData(const DrawCmd &cmd) const
{
	// Push constants (glUniforms) don't need to be unbound
	// Uniform buffers also don't need to be unbound
	// Textures, though theoretically could be unbound, are all
	// managed from a central place and therefore don't need to
	// be unbound
}

void CommandList::ExecuteDrawCmd(const DrawCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	stateCache->SetRenderState(cmd.renderStateHash);
	CHECKERRORS();

	ApplyDrawData(cmd);
	CHECKERRORS();

	PrimitiveType pt = stateCache->GetActiveRenderState().primitiveType;
	if (cmd.inst)
		m_renderer->DrawMeshInstancedInternal(cmd.mesh, cmd.offset, cmd.inst, pt, cmd.numElems);
	else
		m_renderer->DrawMeshInternal(cmd.mesh, cmd.offset, pt, cmd.numElems);

	CleanupDrawData(cmd);
	CHECKERRORS();
}

void CommandList::ExecuteRenderPassCmd(const RenderPassCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	if (cmd.setRenderTarget)
		stateCache->SetRenderTarget(cmd.renderTarget, cmd.extents);

	if (cmd.clearColors || cmd.clearDepth)
		stateCache->ClearBuffers(cmd.clearColors, cmd.clearDepth, cmd.clearColor);

	CHECKERRORS();
}
