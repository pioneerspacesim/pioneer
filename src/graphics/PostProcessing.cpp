// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PostProcessing.h"
#include "PostProcess.h"
#include "RenderTarget.h"
#include "WindowSDL.h"
#include "Renderer.h"
#include "RenderState.h"

namespace Graphics {

PostProcessing::PostProcessing(Renderer *renderer) :
	mtrlFullscreenQuad(nullptr), 
	mRenderer(renderer), 
	rtDevice(nullptr)
{
	assert(mRenderer != nullptr);
	Init();
}

PostProcessing::~PostProcessing()
{
}

void PostProcessing::Init()
{
	// Init quad used for rendering
	VertexArray vertices(ATTRIB_POSITION);

	vertices.Add(vector3f(-1.0f, -1.0f, 0.0f));
	vertices.Add(vector3f( 1.0f, -1.0f, 0.0f));
	vertices.Add(vector3f(-1.0f,  1.0f, 0.0f));
	vertices.Add(vector3f( 1.0f, -1.0f, 0.0f));
	vertices.Add(vector3f( 1.0f,  1.0f, 0.0f));
	vertices.Add(vector3f(-1.0f,  1.0f, 0.0f));

	//Create buffers
	VertexBufferDesc vbd;
	vbd.attrib[0].semantic = ATTRIB_POSITION;
	vbd.attrib[0].format   = ATTRIB_FORMAT_FLOAT3;
	vbd.numVertices = vertices.GetNumVerts();
	vbd.usage = BUFFER_USAGE_STATIC;
	m_vertexBuffer.reset(mRenderer->CreateVertexBuffer(vbd));
	m_vertexBuffer->Populate(vertices);

	// Init postprocessing materials
	MaterialDescriptor tfquad_mtrl_desc;
	tfquad_mtrl_desc.effect = EFFECT_TEXTURED_FULLSCREEN_QUAD;
	mtrlFullscreenQuad.reset(mRenderer->CreateMaterial(tfquad_mtrl_desc));

	// Init render targets
	WindowSDL* window = mRenderer->GetWindow();
	RenderTargetDesc rt_desc(
		window->GetWidth(), window->GetHeight(), 
		TextureFormat::TEXTURE_RGBA_8888, TextureFormat::TEXTURE_DEPTH,
		true);
	rtMain = mRenderer->CreateRenderTarget(rt_desc);

	// Init render state
	RenderStateDesc rsd;
	rsd.blendMode = BlendMode::BLEND_SOLID;
	rsd.depthTest = false;
	rsd.depthWrite = false;
	mRenderState = mRenderer->CreateRenderState(rsd);
}

// rt_device is added so device render target can be changed
void PostProcessing::BeginFrame()
{
	mRenderer->SetRenderTarget(rtMain);
	mRenderer->ClearScreen();
}

void PostProcessing::EndFrame()
{
	mRenderer->SetRenderTarget(rtDevice);
}

// For post processing material:
// Normal pass: 
//		texture0: set to previous pass output (main if it's the first pass)
// Compose pass: (at least 1 pass before it to work)
//		texture0: set to main output always
//		texture1: set to previous output
void PostProcessing::Run(PostProcess* pp)
{
	if(pp == nullptr || pp->GetPassCount() == 0) {
		// No post-processing
		mRenderer->SetRenderTarget(rtDevice);
		mtrlFullscreenQuad->texture0 = rtMain->GetColorTexture();
		mRenderer->DrawBuffer(m_vertexBuffer.get(), mRenderState, mtrlFullscreenQuad.get());
	} else {
		RenderTarget* rt_src = rtMain;
		RenderTarget* rt_dest = 0;
		for(unsigned int i = 0; i < pp->vPasses.size(); ++i) {
			if(i == pp->vPasses.size() - 1) {
				rt_dest = 0;
			} else {
				rt_dest = pp->vPasses[i]->renderTarget.get();
			}
			Material *mtrl = pp->vPasses[i]->material.get();
			mRenderer->SetRenderTarget(rt_dest);
			glDepthMask(GL_FALSE);
			if(pp->vPasses[i]->type == PP_PASS_COMPOSE) {
				mtrl->texture0 = rtMain->GetColorTexture();
				mtrl->texture1 = rt_src->GetColorTexture();
			} else {
				mtrl->texture0 = rt_src->GetColorTexture();
			}
			mRenderer->DrawBuffer(m_vertexBuffer.get(), mRenderState, mtrl);
			rt_src = rt_dest;
		}
	}
}

void PostProcessing::SetDeviceRT(RenderTarget* rt_device)
{
	rtDevice = rt_device;
}

}
