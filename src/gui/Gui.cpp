// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"
#include "graphics/Graphics.h"
#include "graphics/RenderState.h"

namespace Gui {

namespace RawEvents {
	sigc::signal<void, MouseMotionEvent *> onMouseMotion;
	sigc::signal<void, MouseButtonEvent *> onMouseDown;
	sigc::signal<void, MouseButtonEvent *> onMouseUp;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyDown;
	sigc::signal<void, SDL_KeyboardEvent *> onKeyUp;
	sigc::signal<void, SDL_JoyAxisEvent *> onJoyAxisMotion;
	sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonDown;
	sigc::signal<void, SDL_JoyButtonEvent *> onJoyButtonUp;
	sigc::signal<void, SDL_JoyHatEvent *> onJoyHatMotion;
}

static Sint32 lastMouseX, lastMouseY;
void HandleSDLEvent(SDL_Event *event)
{
	PROFILE_SCOPED()
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			lastMouseX = event->button.x;
			lastMouseY = event->button.y;
			Screen::OnClick(&event->button);
			break;
		case SDL_MOUSEBUTTONUP:
			lastMouseX = event->button.x;
			lastMouseY = event->button.y;
			Screen::OnClick(&event->button);
			break;
		case SDL_MOUSEWHEEL: {
			// synthesizing an SDL1.2-style button event for mouse wheels
			SDL_MouseButtonEvent ev;
            ev.type = SDL_MOUSEBUTTONDOWN;
			ev.button = event->wheel.y > 0 ? MouseButtonEvent::BUTTON_WHEELUP : MouseButtonEvent::BUTTON_WHEELDOWN;
			ev.state = SDL_PRESSED;
			ev.x = lastMouseX;
			ev.y = lastMouseY;
			Screen::OnClick(&ev);
			break;
		 }
		case SDL_KEYDOWN:
			Screen::OnKeyDown(&event->key.keysym);
			RawEvents::onKeyDown.emit(&event->key);
			break;
		case SDL_KEYUP:
			Screen::OnKeyUp(&event->key.keysym);
			RawEvents::onKeyUp.emit(&event->key);
			break;
		case SDL_TEXTINPUT:
			Screen::OnTextInput(&event->text);
			break;
		case SDL_MOUSEMOTION:
			lastMouseX = event->motion.x;
			lastMouseY = event->motion.y;
			Screen::OnMouseMotion(&event->motion);
			break;
		case SDL_JOYAXISMOTION:
			RawEvents::onJoyAxisMotion(&event->jaxis);
			break;
		case SDL_JOYBUTTONUP:
			RawEvents::onJoyButtonUp(&event->jbutton);
			break;
		case SDL_JOYBUTTONDOWN:
			RawEvents::onJoyButtonDown(&event->jbutton);
			break;
		case SDL_JOYHATMOTION:
			RawEvents::onJoyHatMotion(&event->jhat);
			break;
	}
}

struct TimerSignal {
	Uint32 goTime;
	sigc::signal<void> sig;
};

static std::list<TimerSignal*> g_timeSignals;

sigc::connection AddTimer(Uint32 ms, sigc::slot<void> slot)
{
	TimerSignal *_s = new TimerSignal;
	_s->goTime = SDL_GetTicks() + ms;
	sigc::connection con = _s->sig.connect(slot);
	g_timeSignals.push_back(_s);
	return con;
}

void Draw()
{
	PROFILE_SCOPED()
	Uint32 t = SDL_GetTicks();
	// also abused like an update() function...
	for (std::list<TimerSignal*>::iterator i = g_timeSignals.begin(); i != g_timeSignals.end();) {
		if (t >= (*i)->goTime) {
			(*i)->sig.emit();
			delete *i;
			i = g_timeSignals.erase(i);
		} else {
			++i;
		}
	}
//	ExpireTimers(t);

	Screen::Draw();
}

void Init(Graphics::Renderer *renderer, int screen_width, int screen_height, int ui_width, int ui_height)
{
	Screen::Init(renderer, screen_width, screen_height, ui_width, ui_height);
}

void Uninit()
{
	std::list<TimerSignal*>::iterator i;
	for (i=g_timeSignals.begin(); i!=g_timeSignals.end(); ++i) delete *i;

	Screen::Uninit();
}

namespace Theme {
	namespace Colors {
		const Color bg(64, 94, 161);
		const Color bgShadow(20, 31, 54);
		const Color tableHeading(178, 178, 255);
	}
	static const float BORDER_WIDTH = 2.0;

	void DrawRect(const vector2f &pos, const vector2f &size, const Color &c, Graphics::RenderState *state)
	{
		Graphics::VertexArray bgArr(Graphics::ATTRIB_POSITION, 4);
		bgArr.Add(vector3f(pos.x,size.y,0));
		bgArr.Add(vector3f(size.x,size.y,0));
		bgArr.Add(vector3f(size.x,pos.y,0));
		bgArr.Add(vector3f(pos.x,pos.y,0));
		Screen::flatColorMaterial->diffuse = c;
		Screen::GetRenderer()->DrawTriangles(&bgArr, state, Screen::flatColorMaterial, Graphics::TRIANGLE_FAN);
	}

	void DrawRoundEdgedRect(const float size[2], float rad, const Color &color, Graphics::RenderState *state)
	{
		static Graphics::VertexArray vts(Graphics::ATTRIB_POSITION);
		vts.Clear();

		const int STEPS = 6;
		if (rad > 0.5f*std::min(size[0], size[1])) rad = 0.5f*std::min(size[0], size[1]);
			// top left
			// bottom left
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*0.5f*i/float(STEPS);
				vts.Add(vector3f(rad - rad*cos(ang), (size[1] - rad) + rad*sin(ang), 0.f));
			}
			// bottom right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*0.5 + M_PI*0.5f*i/float(STEPS);
				vts.Add(vector3f(size[0] - rad - rad*cos(ang), (size[1] - rad) + rad*sin(ang), 0.f));
			}
			// top right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI + M_PI*0.5f*i/float(STEPS);
				vts.Add(vector3f((size[0] - rad) - rad*cos(ang), rad + rad*sin(ang), 0.f));
			}

			// top right
			for (int i=0; i<=STEPS; i++) {
				float ang = M_PI*1.5 + M_PI*0.5f*i/float(STEPS);
				vts.Add(vector3f(rad - rad*cos(ang), rad + rad*sin(ang), 0.f));
			}

		Screen::flatColorMaterial->diffuse = color;
		Screen::GetRenderer()->DrawTriangles(&vts, state, Screen::flatColorMaterial, Graphics::TRIANGLE_FAN);
	}

	struct TPos {
		vector3f pos;
	};

	void DrawHollowRect(const float size[2], const Color &color, Graphics::RenderState *state)
	{
		const vector3f vertices[] = { 
	/* 0 */	vector3f(0,0,0),
	/* 1 */	vector3f(0,size[1],0),
	/* 2 */	vector3f(size[0],size[1],0),
	/* 3 */	vector3f(size[0],0,0),
	/* 4 */	vector3f(BORDER_WIDTH,BORDER_WIDTH,0),
	/* 5 */	vector3f(BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 6 */	vector3f(size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 7 */	vector3f(size[0]-BORDER_WIDTH,BORDER_WIDTH,0)
		};
		const GLushort indices[] = {
			0,1,5, 0,5,4, 0,4,7, 0,7,3,
			3,7,6, 3,6,2, 1,2,6, 1,6,5 };

		// create buffer
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = 8;
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		// Upload data
		// VertexBuffer
		std::unique_ptr<Graphics::VertexBuffer> vb;
		vb.reset(Screen::GetRenderer()->CreateVertexBuffer(vbd));
		TPos* vtxPtr = vb->Map<TPos>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetDesc().stride == sizeof(TPos));
		for(Uint32 i=0 ; i<8 ; i++) {
			vtxPtr[i].pos	= vertices[i];
		}
		vb->Unmap();

		//create index buffer & copy
		std::unique_ptr<Graphics::IndexBuffer> ib;
		ib.reset(Screen::GetRenderer()->CreateIndexBuffer(24, Graphics::BUFFER_USAGE_STATIC));
		Uint16* idxPtr = ib->Map(Graphics::BUFFER_MAP_WRITE);
		for (Uint32 j = 0; j < 24; j++) {
			idxPtr[j] = indices[j];
		}
		ib->Unmap();
		
		Screen::flatColorMaterial->diffuse = color;
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib.get(), state, Screen::flatColorMaterial);
	}

	Graphics::IndexBuffer* CreateIndexBuffer(const GLushort indices[], const Uint32 IndexStart, const Uint32 IndexEnd, const Uint32 NumIndices)
	{
		Graphics::IndexBuffer *ib = Screen::GetRenderer()->CreateIndexBuffer(NumIndices, Graphics::BUFFER_USAGE_STATIC);
		Uint16* idxPtr = ib->Map(Graphics::BUFFER_MAP_WRITE);
		for (Uint32 j = 0; j < NumIndices; j++) {
			idxPtr[j] = indices[j + IndexStart];
		}
		ib->Unmap();

		return ib;
	}

	void DrawIndent(const float size[2], Graphics::RenderState *state)
	{
		const vector3f vertices[] = { 
	/* 0 */	vector3f(0,0,0),
	/* 1 */	vector3f(0,size[1],0),
	/* 2 */	vector3f(size[0],size[1],0),
	/* 3 */	vector3f(size[0],0,0),
	/* 4 */	vector3f(BORDER_WIDTH,BORDER_WIDTH,0),
	/* 5 */	vector3f(BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 6 */	vector3f(size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 7 */	vector3f(size[0]-BORDER_WIDTH,BORDER_WIDTH, 0)
		};
		const GLushort indices[] = {
			0,1,5, 0,5,4, 0,4,7, 0,7,3,
			3,7,6, 3,6,2, 1,2,6, 1,6,5,
			4,5,6, 4,6,7 };

		// create buffer
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = 8;
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		// Upload data
		std::unique_ptr<Graphics::VertexBuffer> vb;
		vb.reset( Screen::GetRenderer()->CreateVertexBuffer(vbd) );
		TPos* vtxPtr = vb->Map<TPos>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetDesc().stride == sizeof(TPos));
		for(Uint32 i=0 ; i<8 ; i++) {
			vtxPtr[i].pos	= vertices[i];
		}
		vb->Unmap();

		// indices
		Uint32 IndexStart = 0;
		Uint32 IndexEnd = 12;
		Uint32 NumIndices = 12;

		std::unique_ptr<Graphics::IndexBuffer> ib[3];
		ib[0].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		IndexStart += NumIndices;
		NumIndices = 12;
		IndexEnd += NumIndices;

		ib[1].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		IndexStart += NumIndices;
		NumIndices = 6;
		IndexEnd += NumIndices;

		ib[2].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		// Draw it!
		Screen::flatColorMaterial->diffuse = Colors::bgShadow;
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[0].get(), state, Screen::flatColorMaterial);
		Screen::flatColorMaterial->diffuse = Color(153,153,153,255);
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[1].get(), state, Screen::flatColorMaterial);
		Screen::flatColorMaterial->diffuse = Colors::bg;
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[2].get(), state, Screen::flatColorMaterial);
	}

	void DrawOutdent(const float size[2], Graphics::RenderState *state)
	{
		const vector3f vertices[] = { 
	/* 0 */	vector3f(0,0,0),
	/* 1 */	vector3f(0,size[1],0),
	/* 2 */	vector3f(size[0],size[1],0),
	/* 3 */	vector3f(size[0],0,0),
	/* 4 */	vector3f(BORDER_WIDTH,BORDER_WIDTH,0),
	/* 5 */	vector3f(BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 6 */	vector3f(size[0]-BORDER_WIDTH,size[1]-BORDER_WIDTH,0),
	/* 7 */	vector3f(size[0]-BORDER_WIDTH,BORDER_WIDTH, 0)
		};
		const GLushort indices[] = {
			0,1,5, 0,5,4, 0,4,7, 0,7,3,
			3,7,6, 3,6,2, 1,2,6, 1,6,5,
			4,5,6, 4,6,7 };

		// create buffer
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = 8;
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;

		// Upload data
		std::unique_ptr<Graphics::VertexBuffer> vb;
		vb.reset( Screen::GetRenderer()->CreateVertexBuffer(vbd) );
		TPos* vtxPtr = vb->Map<TPos>(Graphics::BUFFER_MAP_WRITE);
		assert(vb->GetDesc().stride == sizeof(TPos));
		for(Uint32 i=0 ; i<8 ; i++) {
			vtxPtr[i].pos	= vertices[i];
		}
		vb->Unmap();

		// indices
		Uint32 IndexStart = 0;
		Uint32 IndexEnd = 12;
		Uint32 NumIndices = 12;

		std::unique_ptr<Graphics::IndexBuffer> ib[3];
		ib[0].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		IndexStart += NumIndices;
		NumIndices = 12;
		IndexEnd += NumIndices;

		ib[1].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		IndexStart += NumIndices;
		NumIndices = 6;
		IndexEnd += NumIndices;

		ib[2].reset(CreateIndexBuffer(indices, IndexStart, IndexEnd, NumIndices));

		// Draw it!
		Screen::flatColorMaterial->diffuse = Color(153,153,153,255);
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[0].get(), state, Screen::flatColorMaterial);
		Screen::flatColorMaterial->diffuse = Colors::bgShadow;
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[1].get(), state, Screen::flatColorMaterial);
		Screen::flatColorMaterial->diffuse = Colors::bg;
		Screen::GetRenderer()->DrawBufferIndexed(vb.get(), ib[2].get(), state, Screen::flatColorMaterial);
	}
}

}
