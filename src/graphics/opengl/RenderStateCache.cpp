// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RenderStateCache.h"
#include "Program.h"
#include "UniformBuffer.h"
#include "core/Log.h"
#include "graphics/opengl/RenderTargetGL.h"
#include "graphics/opengl/TextureGL.h"

using namespace Graphics::OGL;
using RenderStateDesc = Graphics::RenderStateDesc;

const RenderStateDesc &RenderStateCache::GetRenderState(size_t hash) const
{
	for (auto &pair : m_stateDescCache)
		if (pair.first == hash)
			return pair.second;

	Log::Warning("Attempt to get render state for unknown hash {}! Returning current state.", hash);
	return m_activeRenderState;
}

void RenderStateCache::SetRenderState(size_t hash)
{
	if (hash == m_activeRenderStateHash)
		return;

	const RenderStateDesc &rsd = GetRenderState(hash);

	if (rsd.blendMode != m_activeRenderState.blendMode || !m_activeRenderStateHash) {
		switch (rsd.blendMode) {
		case BLEND_SOLID:
			glDisable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ZERO);
			break;
		case BLEND_ADDITIVE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case BLEND_ALPHA:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_ALPHA_ONE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BLEND_ALPHA_PREMULT:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_SET_ALPHA:
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
			break;
		case BLEND_DEST_ALPHA:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
		default:
			break;
		}
	}

	if (rsd.cullMode != m_activeRenderState.cullMode || !m_activeRenderStateHash) {
		if (rsd.cullMode == CULL_BACK) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		} else if (rsd.cullMode == CULL_FRONT) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	if (rsd.depthTest != m_activeRenderState.depthTest || !m_activeRenderStateHash) {
		if (rsd.depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	if (rsd.depthWrite != m_activeRenderState.depthWrite || !m_activeRenderStateHash) {
		if (rsd.depthWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}

	if (rsd.scissorTest != m_activeRenderState.scissorTest || !m_activeRenderStateHash) {
		if (rsd.scissorTest)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}

	m_activeRenderState = rsd;
	m_activeRenderStateHash = hash;
}

static size_t HashRenderStateDesc(const RenderStateDesc &desc)
{
	// Can't directly pass RenderStateDesc& to lookup3_hashlittle, because
	// it (most likely) has padding bytes, and those bytes are uninitialized,
	// thereby arbitrarily affecting the hash output.
	// (We used to do this and valgrind complained).
	uint32_t words[5] = {
		desc.blendMode,
		desc.cullMode,
		desc.primitiveType,
		desc.depthTest,
		desc.depthWrite,
	};
	uint32_t a = 0, b = 0;
	lookup3_hashword2(words, 5, &a, &b);
	return size_t(a) | (size_t(b) << 32);
}

size_t RenderStateCache::InternRenderState(const RenderStateDesc &rsd)
{
	size_t hash = HashRenderStateDesc(rsd);
	for (auto &pair : m_stateDescCache)
		if (pair.first == hash)
			return pair.first;

	m_stateDescCache.emplace_back(hash, rsd);
	return hash;
}

void RenderStateCache::ResetFrame()
{
	// Textures might be deleted between frames while they're still referenced in cache
	// so we clear the texture cache
	for (uint32_t idx = 0; idx < m_textureCache.size(); idx++)
		SetTexture(idx, nullptr);

	m_activeRenderStateHash = 0;
	m_activeProgram = 0;
}

void RenderStateCache::SetTexture(uint32_t index, TextureGL *texture)
{
	if (index >= m_textureCache.size())
		m_textureCache.resize(index + 1, nullptr);

	// Don't do anything if the texture is the same as what was last bound.
	TextureGL *current = m_textureCache[index];
	if (current == texture)
		return;

	glActiveTexture(GL_TEXTURE0 + index);

	// Unbind the previous texture if we're changing texture targets or clearing the texture binding
	if (current && (!texture || texture->GetTarget() != current->GetTarget()))
		current->Unbind();

	if (texture)
		texture->Bind();

	m_textureCache[index] = texture;
}

void RenderStateCache::SetBufferBinding(uint32_t index, UniformBufferBinding &binding)
{
	if (index >= m_bufferCache.size())
		m_bufferCache.resize(index + 1, UniformBufferBinding{ nullptr, 0, 0 });

	UniformBufferBinding &current = m_bufferCache[index];
	if (current == binding)
		return;

	if (binding.buffer)
		binding.buffer->BindRange(index, binding.offset, binding.size);
	current = binding;
}

void RenderStateCache::SetProgram(Program *program)
{
	GLuint newProgram = (program ? program->GetProgramID() : 0);
	if (m_activeProgram == newProgram)
		return;

	m_activeProgram = newProgram;
	glUseProgram(m_activeProgram);
}

void RenderStateCache::SetRenderTarget(RenderTarget *target)
{
	if (m_activeRT != target) {
		if (m_activeRT)
			m_activeRT->Unbind();
		if (target)
			target->Bind();
	}
}

void RenderStateCache::SetRenderTarget(RenderTarget *target, ViewportExtents extents)
{
	SetRenderTarget(target);

	if (extents != m_currentExtents) {
		m_currentExtents = extents;
		glViewport(extents.x, extents.y, extents.w, extents.h);
	}
}

void RenderStateCache::SetScissor(ViewportExtents scissor)
{
	if (scissor == m_currentScissor)
		return;

	glScissor(scissor.x, scissor.y, scissor.w, scissor.h);
	m_currentScissor = scissor;
}

void RenderStateCache::ClearBuffers(bool colorBuffer, bool depthBuffer, Color clearColor)
{
	uint32_t flags = (colorBuffer ? GL_COLOR_BUFFER_BIT : 0) | (depthBuffer ? GL_DEPTH_BUFFER_BIT : 0);
	if (!colorBuffer && !depthBuffer)
		return;

	if (m_activeRenderState.scissorTest)
		glDisable(GL_SCISSOR_TEST);

	if (colorBuffer)
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

	if (depthBuffer) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClear(flags);

		if (!m_activeRenderState.depthTest)
			glDisable(GL_DEPTH_TEST);
		if (!m_activeRenderState.depthWrite)
			glDepthMask(GL_FALSE);
	} else {
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (m_activeRenderState.scissorTest)
		glEnable(GL_SCISSOR_TEST);
}
