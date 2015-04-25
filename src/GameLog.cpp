#include "GameLog.h"
#include "StringF.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

const Uint32 MESSAGE_TIMEOUT  = 8000;
const Uint32 FADE_TIME  = 1000;
const Uint32 FADE_AFTER = MESSAGE_TIMEOUT - FADE_TIME;
const Uint8 MAX_MESSAGES = 6;

GameLog::Message::Message(const std::string &m, Uint32 t)
	: msg(m), time(t), m_prevAlpha(0)
{}

GameLog::GameLog(RefCountedPtr<Text::TextureFont> font, vector2f scrSize)
: m_font(font)
, m_screenSize(scrSize)
, m_prevMessages(0)
{
	m_lineHeight = m_font->GetHeight();

	// position just above cpanel
	m_offset.x = scrSize.x / 800.f * 170.f;
	m_offset.y = scrSize.y / 600.f * 505.f;
}

void GameLog::Add(const std::string &msg)
{
	m_messages.push_back(Message(msg, 0));

	while (m_messages.size() > MAX_MESSAGES) m_messages.pop_front();
}

void GameLog::Add(const std::string &from, const std::string &msg)
{
	if(from.empty())
		Add(msg);
	else
		Add(stringf("%0: %1", from, msg));
}

void GameLog::Update(bool paused)
{
	static Uint32 prevTime = 0;
	Uint32 now = SDL_GetTicks();
	Uint32 elapsed = now - prevTime;
	prevTime = now;

	//age messages
	if (!paused) {
		for (auto it = m_messages.begin(); it != m_messages.end();) {
			it->time += elapsed;

			if (it->time >= MESSAGE_TIMEOUT)
				m_messages.erase(it++);
			else
				++it;
		}
	}
}

void GameLog::DrawHudMessages(Graphics::Renderer *r)
{
	Graphics::Renderer::StateTicket ticket(r);
	Graphics::RenderStateDesc rsd;
	rsd.depthTest  = false;
	rsd.depthWrite = false;
	Graphics::RenderState *prsd = r->CreateRenderState(rsd);

	//I'd rather render this as one string, but then we can't
	//have per-line fade - markup doesn't support alpha
	r->SetOrthographicProjection(0, m_screenSize.x, m_screenSize.y, 0, -1, 1);
	r->SetTransform(matrix4x4f::Identity());
	r->SetRenderState(prsd);

	const Color &c = Color::WHITE;

	// (re)build buffers
	const size_t numMessages = m_messages.size();
	const bool bRefresh = (numMessages != m_prevMessages);
	// update message loop
	float y = 0;
	for (auto it = m_messages.rbegin(), itEnd = m_messages.rend(); it != itEnd; ++it) {
		float alpha = 1.f;
		if (it->time > FADE_AFTER) {
			alpha = 1.0f - (float(it->time - FADE_AFTER) / float(FADE_TIME));
		}
		Uint32 iAlpha(Clamp(Uint32(alpha*255), 0U, 255U));
		const Color textColour(c.r, c.g, c.b, iAlpha);

		// update only if things have changed or the buffer does not exist
		if (bRefresh || !it->m_vb.Valid() || iAlpha != it->m_prevAlpha || !it->m_prevoffset.ExactlyEqual(m_offset))
		{
			it->m_prevAlpha = iAlpha;
			it->m_prevoffset = m_offset;
			Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);
			m_font->PopulateString(va, it->msg.c_str(), m_offset.x, m_offset.y + y, textColour);
			it->m_vb.Reset( m_font->CreateVertexBuffer(va) );
		}

		m_font->RenderBuffer( it->m_vb.Get() );
		y -= m_lineHeight;
	}
	m_prevMessages = numMessages;
}
