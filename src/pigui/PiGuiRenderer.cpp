// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGuiRenderer.h"

#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Texture.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "profiler/Profiler.h"

#include "imgui/imgui.h"

#include <cstring>

using namespace PiGui;

static constexpr size_t s_textureName = "texture0"_hash;
static constexpr size_t s_vertexDepthName = "vertexDepth"_hash;

Graphics::TextureFormat GetTextureFormat(ImTextureFormat fmt)
{
	switch(fmt) {
		case ImTextureFormat_RGBA32: return Graphics::TEXTURE_RGBA_8888;
		case ImTextureFormat_Alpha8: return Graphics::TEXTURE_R8;
		default: assert(0);
	}
}

InstanceRenderer::InstanceRenderer(Graphics::Renderer *r) :
	m_renderer(r)
{}

void InstanceRenderer::Initialize()
{
	ImGuiIO &io = ImGui::GetIO();
	io.BackendRendererName = "Pioneer Renderer";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

	Graphics::VertexFormatDesc vfmt = {};
	vfmt.attribs[0] = { Graphics::ATTRIB_FORMAT_FLOAT2, 0, 0, offsetof(ImDrawVert, pos) };
	vfmt.attribs[1] = { Graphics::ATTRIB_FORMAT_FLOAT2, 3, 0, offsetof(ImDrawVert, uv)  };
	vfmt.attribs[2] = { Graphics::ATTRIB_FORMAT_UBYTE4, 2, 0, offsetof(ImDrawVert, col) };

	vfmt.bindings[0] = { sizeof(ImDrawVert), true, Graphics::ATTRIB_RATE_NORMAL };

	m_vtxBuffer.reset(m_renderer->CreateVertexBuffer(Graphics::BUFFER_USAGE_DYNAMIC, 0, vfmt.bindings[0].stride));
	m_idxBuffer.reset(m_renderer->CreateIndexBuffer(0, Graphics::BUFFER_USAGE_DYNAMIC, Graphics::INDEX_BUFFER_16BIT));

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.cullMode = Graphics::CULL_NONE;
	rsd.depthTest = true;
	rsd.depthWrite = false;
	rsd.scissorTest = true;

	Graphics::MaterialDescriptor mDesc;
	mDesc.textures = 1;
	mDesc.alphaTest = 1;

	m_material.reset(m_renderer->CreateMaterial("ui", mDesc, rsd, vfmt));
}

void InstanceRenderer::Shutdown()
{
	for (auto *tex : ImGui::GetPlatformIO().Textures)
	{
		if (tex->RefCount == 1 && tex->TexID != ImTextureID_Invalid) {
			delete reinterpret_cast<Graphics::Texture *>(tex->TexID);

			tex->SetTexID(ImTextureID_Invalid);
			tex->SetStatus(ImTextureStatus_Destroyed);
		}
	}

	m_vtxBuffer.reset();
	m_idxBuffer.reset();
	m_material.reset();
}

void InstanceRenderer::RenderDrawData(ImDrawData *draw_data)
{
	for (auto *tex : *draw_data->Textures) {
		if (tex->Status != ImTextureStatus_OK)
			UpdateFontTexture(tex);
	}

	float L = draw_data->DisplayPos.x;
	float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	float T = draw_data->DisplayPos.y;
	float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

	m_renderer->SetTransform(matrix4x4f::Identity);
	m_renderer->SetProjection(matrix4x4f::OrthoFrustum(L, R, B, T, -1.0, 0.0));

	RenderDrawData(draw_data, m_material.get());
}

void InstanceRenderer::RenderDrawData(ImDrawData *draw_data, Graphics::Material* material)
{
	PROFILE_SCOPED()
	Graphics::Renderer::StateTicket st(m_renderer);

	ImGuiIO &io = ImGui::GetIO();
	int fb_width = (int)(fabs(draw_data->DisplaySize.x) * io.DisplayFramebufferScale.x);
	int fb_height = (int)(fabs(draw_data->DisplaySize.y) * io.DisplayFramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// we're going to throw all of the vertex and index data straight to the GPU
	// in a single buffer for each, right before we begin executing commands.
	// This should make optimal use of transfer resources.
	std::vector<ImDrawVert> vtxStagingBuffer;
	vtxStagingBuffer.reserve(1024);
	std::vector<ImDrawIdx> idxStagingBuffer;
	idxStagingBuffer.reserve(1024);

	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];

		// coalesce vertex and index data into a single buffer upload
		auto &imVtxBuffer = cmd_list->VtxBuffer;
		const size_t vtxOffset = vtxStagingBuffer.size();
		vtxStagingBuffer.reserve(vtxOffset + imVtxBuffer.Size);

		auto &imIdxBuffer = cmd_list->IdxBuffer;
		const size_t idxOffset = idxStagingBuffer.size();
		idxStagingBuffer.reserve(idxOffset + imIdxBuffer.Size);

		// write this command list's data to the tail of the staging array
		vtxStagingBuffer.insert(vtxStagingBuffer.end(), imVtxBuffer.Data, imVtxBuffer.Data + imVtxBuffer.Size);
		idxStagingBuffer.insert(idxStagingBuffer.end(), imIdxBuffer.Data, imIdxBuffer.Data + imIdxBuffer.Size);

		// Generate renderer commands for each draw command in the command buffer list.
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			else {
				ImVec2 pos = draw_data->DisplayPos;
				ImVec4 clip_rect = pcmd->ClipRect - ImVec4(pos.x, pos.y, pos.x, pos.y);

				// do a simple screen bounds test
				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.f && clip_rect.w >= 0.f) {
					Graphics::ViewportExtents vp(clip_rect.x, (fb_height - clip_rect.w), (clip_rect.z - clip_rect.x), (clip_rect.w - clip_rect.y));
					m_renderer->SetScissor(vp);

					material->SetTexture(s_textureName, reinterpret_cast<Graphics::Texture *>(pcmd->GetTexID()));
					material->SetPushConstant(s_vertexDepthName, pcmd->PrimDepth);

					uint32_t startVtx = vtxOffset + pcmd->VtxOffset;
					uint32_t startIdx = idxOffset + pcmd->IdxOffset;
					m_renderer->Draw({ { m_vtxBuffer.get(), startVtx } }, { m_idxBuffer.get(), startIdx }, material, pcmd->ElemCount);
				}
			}
		}
	}

	// so long as we haven't issued FlushCommandBuffers() yet, we're perfectly fine to do this upload out-of-order.
	m_vtxBuffer->BufferData(vtxStagingBuffer.size() * sizeof(ImDrawVert), vtxStagingBuffer.data());
	m_idxBuffer->BufferData(idxStagingBuffer.size() * sizeof(ImDrawIdx), idxStagingBuffer.data());

	m_renderer->FlushCommandBuffers();
}

void InstanceRenderer::UpdateFontTexture(ImTextureData *tex)
{
	PROFILE_SCOPED();

	if (tex->Status == ImTextureStatus_WantCreate) {
		// Create a new texture (pointer is owned by ImGui)

		Graphics::TextureDescriptor texDesc(
			GetTextureFormat(tex->Format),
			vector3f(tex->Width, tex->Height, 0),
			Graphics::LINEAR_REPEAT,
			false, false, false, 0,
			Graphics::TEXTURE_2D);

		Graphics::Texture *ptr = m_renderer->CreateTexture(texDesc);
		assert(ptr);

		ptr->Update(tex->GetPixels(), texDesc.dataSize, texDesc.format);

		tex->SetTexID(ImTextureID(ptr));
		tex->SetStatus(ImTextureStatus_OK);
	}

	if (tex->Status == ImTextureStatus_WantUpdates) {
		// Upload data to an existing texture

		auto *ptr = reinterpret_cast<Graphics::Texture *>(tex->TexID);

		// Texture::Update expects the source region to be fully contiguous
		uint8_t *texel_data = new uint8_t[tex->UpdateRect.w * tex->UpdateRect.h * tex->BytesPerPixel];

		uint8_t *blit_ptr = texel_data;
		for (int y = 0; y < tex->UpdateRect.h; y++) {
			size_t width = tex->UpdateRect.w * tex->BytesPerPixel;
			std::memcpy(blit_ptr, tex->GetPixelsAt(tex->UpdateRect.x, tex->UpdateRect.y + y), width);
			blit_ptr += width;
		}

		ptr->Update(texel_data,
			vector2f(tex->UpdateRect.x, tex->UpdateRect.y),
			vector3f(tex->UpdateRect.w, tex->UpdateRect.h, 0),
			GetTextureFormat(tex->Format));

		delete[] texel_data;

		tex->SetStatus(ImTextureStatus_OK);
	}

	if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
	{
		// Release the texture
		delete reinterpret_cast<Graphics::Texture *>(tex->TexID);

		tex->SetTexID(ImTextureID_Invalid);
		tex->SetStatus(ImTextureStatus_Destroyed);
	}
}
