// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CommandBufferGL.h"
#include "MaterialGL.h"
#include "Program.h"
#include "RenderStateCache.h"
#include "RendererGL.h"
#include "RenderTargetGL.h"
#include "Shader.h"
#include "TextureGL.h"
#include "UniformBuffer.h"
#include "VertexBufferGL.h"

#include "graphics/VertexBuffer.h"
#include "profiler/Profiler.h"

using namespace Graphics::OGL;

static GLuint get_element_size(IndexBuffer *idx)
{
	return idx->GetElementSize() == Graphics::INDEX_BUFFER_16BIT ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

void CommandList::AddDrawMeshCmd(Graphics::MeshObject *mesh, Graphics::Material *material)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");
	OGL::Material *mat = static_cast<OGL::Material *>(material);

	DrawMeshCmd cmd{};
	cmd.mesh = static_cast<OGL::MeshObject *>(mesh);
	cmd.elementCount = cmd.mesh->m_idxBuffer ? cmd.mesh->m_idxBuffer->GetIndexCount() : cmd.mesh->m_vtxBuffer->GetSize();

	cmd.program = mat->EvaluateVariant();
	cmd.renderStateHash = mat->m_renderStateHash;
	cmd.drawData = SetupMaterialData(mat);
	cmd.vertexState = mat->m_vertexState;

	m_drawCmds.emplace_back(std::move(cmd));
}

void CommandList::AddDrawBuffersCmd(Span<const BufferBinding<Graphics::VertexBuffer>> vtxBuffers, BufferBinding<Graphics::IndexBuffer> idxBuffer, Graphics::Material *material, uint32_t elementCount, uint32_t instanceCount)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");
	assert(vtxBuffers.size() <= 4 && "At most 4 vertex buffers may be bound to a draw command.");
	assert(instanceCount < (1 << 29) && "Draw instance count limit is 2^29. (what are you doing that's drawing more than 2^29 instances?)");
	OGL::Material *mat = static_cast<OGL::Material *>(material);

	uint32_t numVtxBuffers = std::min<uint32_t>(vtxBuffers.size(), 4);

	DrawBuffersCmd cmd{};
	cmd.numVtxBuffers = numVtxBuffers - 1;
	cmd.idxBuffer = idxBuffer.buffer ? 1 : 0;
	cmd.instanceCount = instanceCount;
	cmd.elementCount = elementCount;
	cmd.drawData = SetupMaterialData(mat, numVtxBuffers + cmd.idxBuffer);

	uint32_t *offsetPtr = cmd.getBufferOffsetsPtr();

	// Store vertex/index buffer bindings to the draw stream and record offsets into those buffers
	// in "basic machine units" (bytes) for OpenGL to consume.

	for (size_t idx = 0; idx < vtxBuffers.size(); idx++) {
		reinterpret_cast<OGL::VertexBuffer **>(cmd.drawData)[idx] = static_cast<OGL::VertexBuffer *>(vtxBuffers[idx].buffer);
		offsetPtr[idx] = vtxBuffers[idx].offset * vtxBuffers[idx].buffer->GetStride();
	}

	if (cmd.idxBuffer) {
		uint32_t indexSize = idxBuffer.buffer->GetElementSize() == INDEX_BUFFER_16BIT ? sizeof(uint16_t) : sizeof(uint32_t);
		reinterpret_cast<OGL::IndexBuffer **>(cmd.drawData)[numVtxBuffers] = static_cast<OGL::IndexBuffer *>(idxBuffer.buffer);
		offsetPtr[numVtxBuffers] = idxBuffer.offset * indexSize;
	}

	cmd.program = mat->EvaluateVariant();
	cmd.renderStateHash = mat->m_renderStateHash;
	cmd.vertexState = mat->m_vertexState;

	m_drawCmds.emplace_back(std::move(cmd));
}

// RenderPass commands can be combined as long as they're setting the same render target
// because viewport does not affect scissor or clear state
void CommandList::AddRenderPassCmd(RenderTarget *renderTarget, ViewportExtents extents)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	RenderPassCmd *lastCmd = IsEmpty() ? nullptr : std::get_if<RenderPassCmd>(&m_drawCmds.back());
	if (!lastCmd || !lastCmd->setRenderTarget || lastCmd->renderTarget != renderTarget) {
		RenderPassCmd cmd{};
		cmd.renderTarget = renderTarget;
		cmd.extents = extents;
		cmd.setRenderTarget = true;
		cmd.setScissor = false;
		cmd.clearColors = false;
		cmd.clearDepth = false;

		m_drawCmds.emplace_back(std::move(cmd));
	} else {
		lastCmd->renderTarget = renderTarget;
		lastCmd->extents = extents;
		lastCmd->setRenderTarget = true;
	}
}

// Scissor commands can be combined with any previous render pass cmd
// (because scissor test will be disabled during render target clears)
void CommandList::AddScissorCmd(ViewportExtents scissor)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	RenderPassCmd *lastCmd = IsEmpty() ? nullptr : std::get_if<RenderPassCmd>(&m_drawCmds.back());
	if (!lastCmd) {
		RenderPassCmd cmd{};
		cmd.scissor = scissor;
		cmd.setRenderTarget = false;
		cmd.setScissor = true;
		cmd.clearColors = false;
		cmd.clearDepth = false;

		m_drawCmds.emplace_back(std::move(cmd));
	} else {
		lastCmd->setScissor = true;
		lastCmd->scissor = scissor;
	}
}

// Clear commands can be combined with any previous render pass command
void CommandList::AddClearCmd(bool clearColors, bool clearDepth, Color color)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	RenderPassCmd *lastCmd = IsEmpty() ? nullptr : std::get_if<RenderPassCmd>(&m_drawCmds.back());

	if (!lastCmd) {
		RenderPassCmd cmd{};
		cmd.setRenderTarget = false;
		cmd.setScissor = false;
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

void CommandList::AddBlitRenderTargetCmd(
	Graphics::RenderTarget *src, Graphics::RenderTarget *dst,
	const ViewportExtents &srcExtents,
	const ViewportExtents &dstExtents,
	bool resolveMSAA, bool blitDepthBuffer,
	bool linearFilter)
{
	assert(!m_executing && "Attempt to append to a command list while it's being executed!");

	if (resolveMSAA) {
		assert(srcExtents.w == dstExtents.w && srcExtents.h == dstExtents.h &&
			"Cannot scale a framebuffer while performing MSAA resolve!");
	}

	BlitRenderTargetCmd cmd{};
	cmd.srcTarget = static_cast<OGL::RenderTarget *>(src);
	cmd.dstTarget = static_cast<OGL::RenderTarget *>(dst);
	cmd.srcExtents = srcExtents;
	cmd.dstExtents = dstExtents;
	cmd.resolveMSAA = resolveMSAA;
	cmd.blitDepthBuffer = blitDepthBuffer;
	cmd.linearFilter = linearFilter;

	m_drawCmds.emplace_back(std::move(cmd));
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

char *CommandList::AllocDrawData(const Shader *shader, uint32_t numBuffers)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	size_t bufferSize = align<8>(shader->GetNumBufferBindings() * sizeof(BufferBinding<UniformBuffer>));
	size_t textureSize = align<8>(shader->GetNumTextureBindings() * sizeof(Texture *));
	size_t bindingSize = align<8>(numBuffers * (sizeof(VertexBuffer *) + sizeof(uint32_t)));
	size_t totalSize = constantSize + bufferSize + textureSize + bindingSize;

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

Graphics::BufferBinding<UniformBuffer> *CommandList::getBufferBindings(const Shader *shader, char *data)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	return reinterpret_cast<BufferBinding<UniformBuffer> *>(data + constantSize);
}

TextureGL **CommandList::getTextureBindings(const Shader *shader, char *data)
{
	size_t constantSize = align<8>(shader->GetConstantStorageSize());
	size_t bufferSize = align<8>(shader->GetNumBufferBindings() * sizeof(BufferBinding<UniformBuffer>));
	return reinterpret_cast<TextureGL **>(data + constantSize + bufferSize);
}

char *CommandList::SetupMaterialData(OGL::Material *mat, uint32_t numBuffers)
{
	PROFILE_SCOPED()
	mat->UpdateDrawData();
	const Shader *s = mat->GetShader();

	char *alloc = AllocDrawData(s, numBuffers);
	char *dataStart = alloc + align<8>(numBuffers * (sizeof(VertexBuffer *) + sizeof(uint32_t)));
	if (mat->m_pushConstants) {
		memcpy(dataStart, mat->m_pushConstants.get(), s->GetConstantStorageSize());
	}

	BufferBinding<UniformBuffer> *buffers = getBufferBindings(s, dataStart);
	for (size_t index = 0; index < s->GetNumBufferBindings(); index++) {
		buffers[index] = mat->m_bufferBindings[index];
	}

	TextureGL **textures = getTextureBindings(s, dataStart);
	for (size_t index = 0; index < s->GetNumTextureBindings(); index++) {
		textures[index] = static_cast<TextureGL *>(mat->m_textureBindings[index]);
	}

	return alloc;
}

void CommandList::ApplyDrawData(Program *program, char *drawData) const
{
	PROFILE_SCOPED();
	const Shader *shader = program->GetShader();
	RenderStateCache *state = m_renderer->GetStateCache();

	state->SetProgram(program);

	BufferBinding<UniformBuffer> *buffers = getBufferBindings(shader, drawData);
	for (auto &info : shader->GetBufferBindings())
		state->SetBufferBinding(info.binding, buffers[info.index]);

	TextureGL **textures = getTextureBindings(shader, drawData);
	for (auto &info : shader->GetTextureBindings())
		state->SetTexture(info.binding, textures[info.index]);

	for (auto &info : shader->GetPushConstantBindings()) {
		GLuint location = program->GetConstantLocation(info.index);
		if (location == GL_INVALID_INDEX)
			continue;

		Uniform setter(location);
		switch (info.format) {
		case ConstantDataFormat::DATA_FORMAT_INT:
			setter.Set(*reinterpret_cast<int *>(drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT:
			setter.Set(*reinterpret_cast<float *>(drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT3:
			setter.Set(*reinterpret_cast<vector3f *>(drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_FLOAT4:
			setter.Set(*reinterpret_cast<Color4f *>(drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_MAT3:
			setter.Set(*reinterpret_cast<matrix3x3f *>(drawData + info.offset));
			break;
		case ConstantDataFormat::DATA_FORMAT_MAT4:
			setter.Set(*reinterpret_cast<matrix4x4f *>(drawData + info.offset));
			break;
		default:
			assert(false);
			break;
		}
	}
}

void CommandList::ExecuteDrawMeshCmd(const DrawMeshCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	stateCache->SetRenderState(cmd.renderStateHash);
	CHECKERRORS();

	ApplyDrawData(cmd.program, cmd.drawData);
	CHECKERRORS();

	PrimitiveType pt = stateCache->GetActiveRenderState().primitiveType;

	glBindVertexArray(cmd.mesh->GetVertexArrayObject());
	OGL::IndexBuffer *idxBuffer = cmd.mesh->m_idxBuffer.Get();

	if (idxBuffer) {
		glDrawElements(pt, cmd.elementCount, get_element_size(idxBuffer), nullptr);
	} else
		glDrawArrays(pt, 0, cmd.elementCount);

	m_renderer->RecordDrawStats(pt, cmd.elementCount, 1);
	CHECKERRORS();
}

void CommandList::ExecuteDrawBuffersCmd(const DrawBuffersCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	stateCache->SetRenderState(cmd.renderStateHash);
	CHECKERRORS();

	uint32_t numVtxBuffers = 1 + cmd.numVtxBuffers;
	size_t drawDataOffset = align<8>((sizeof(VertexBuffer *) + sizeof(uint32_t)) * (numVtxBuffers + cmd.idxBuffer));
	ApplyDrawData(cmd.program, cmd.drawData + drawDataOffset);
	CHECKERRORS();

	uint32_t *offsets = cmd.getBufferOffsetsPtr();
	Span<VertexBuffer *> vtxBuffers = { reinterpret_cast<VertexBuffer **>(cmd.drawData), numVtxBuffers };
	IndexBuffer *idxBuffer = cmd.idxBuffer ? reinterpret_cast<IndexBuffer **>(cmd.drawData)[numVtxBuffers] : nullptr;

	PrimitiveType pt = stateCache->GetActiveRenderState().primitiveType;

	// Bind the VAO containing all vertex attribute assignments
	glBindVertexArray(cmd.vertexState);
	// Bind (or unbind) the element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer ? idxBuffer->GetBuffer() : 0);

	// Bind all vertex buffers with specified offsets
	for (size_t idx = 0; idx < vtxBuffers.size(); idx++) {
		glBindVertexBuffer(idx, vtxBuffers[idx]->GetBuffer(), offsets[idx], vtxBuffers[idx]->GetStride());
	}

	if (idxBuffer) {
		const void *idxOffset = reinterpret_cast<void *>(offsets[numVtxBuffers]);
		if (cmd.instanceCount > 1)
			glDrawElementsInstanced(pt, cmd.elementCount, get_element_size(idxBuffer), idxOffset, cmd.instanceCount);
		else
			glDrawElements(pt, cmd.elementCount, get_element_size(idxBuffer), idxOffset);
	} else {
		if (cmd.instanceCount > 1)
			glDrawArraysInstanced(pt, 0, cmd.elementCount, cmd.instanceCount);
		else
			glDrawArrays(pt, 0, cmd.elementCount);
	}

	m_renderer->RecordDrawStats(pt, cmd.elementCount, cmd.instanceCount);

	CHECKERRORS();
}

void CommandList::ExecuteRenderPassCmd(const RenderPassCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	stateCache->SetProgram(nullptr);

	if (cmd.setRenderTarget)
		stateCache->SetRenderTarget(cmd.renderTarget, cmd.extents);

	if (cmd.setScissor)
		stateCache->SetScissor(cmd.scissor);

	if (cmd.clearColors || cmd.clearDepth)
		stateCache->ClearBuffers(cmd.clearColors, cmd.clearDepth, cmd.clearColor);

	CHECKERRORS();
}

void CommandList::ExecuteBlitRenderTargetCmd(const BlitRenderTargetCmd &cmd)
{
	RenderStateCache *stateCache = m_renderer->GetStateCache();
	stateCache->SetProgram(nullptr);

	// invalidate cached render target state
	stateCache->SetRenderTarget(nullptr);

	if (cmd.srcTarget)
		cmd.srcTarget->Bind(RenderTarget::READ);

	// dstTarget can be null if blitting to the window implicit backbuffer
	if (cmd.dstTarget)
		cmd.dstTarget->Bind(RenderTarget::DRAW);

	int mask = GL_COLOR_BUFFER_BIT | (cmd.blitDepthBuffer ? GL_DEPTH_BUFFER_BIT : 0);

	glBlitFramebuffer(
		cmd.srcExtents.x, cmd.srcExtents.y, cmd.srcExtents.x + cmd.srcExtents.w, cmd.srcExtents.y + cmd.srcExtents.h,
		cmd.dstExtents.x, cmd.dstExtents.y, cmd.dstExtents.x + cmd.dstExtents.w, cmd.dstExtents.y + cmd.dstExtents.h,
		mask, cmd.linearFilter ? GL_LINEAR : GL_NEAREST);

	if (cmd.srcTarget)
		cmd.srcTarget->Unbind(RenderTarget::READ);

	// dstTarget can be null if blitting to the window implicit backbuffer
	if (cmd.dstTarget)
		cmd.dstTarget->Unbind(RenderTarget::DRAW);

	CHECKERRORS();

}
