#include "RocketManager.h"
#include "libs.h"

#include "Rocket/Core/SystemInterface.h"
#include "Rocket/Core/RenderInterface.h"


// RGBA pixel format for converting textures
// XXX little-endian. if we ever have a port to a big-endian arch, invert
//     shift and mask
// XXX texture conversion is theoretically needed anywhere where IMG_Load is
//     used. this structure and the conversion stuff in LoadTexture need to be
//     generalised out
static SDL_PixelFormat rgba_pixfmt = {
	0,                                  // palette
	32,                                 // bits per pixel
	4,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	24, 16, 8, 0,                       // RGBA shift
	0xff, 0xff00, 0xff0000, 0xff000000, // RGBA mask
	0,                                  // colour key
	0                                   // alpha
};


// map of SDL SDLKey -> Rocket KeyIdentifier
struct Keymap {
	SDLKey                             sdl;
	Rocket::Core::Input::KeyIdentifier rocket;
};
static const Keymap keymap[] = {
	{ SDLK_BACKSPACE,    Rocket::Core::Input::KI_BACK },
	{ SDLK_TAB,          Rocket::Core::Input::KI_TAB },
	{ SDLK_CLEAR,        Rocket::Core::Input::KI_CLEAR },
	{ SDLK_RETURN,       Rocket::Core::Input::KI_RETURN },
	{ SDLK_PAUSE,        Rocket::Core::Input::KI_PAUSE },
	{ SDLK_ESCAPE,       Rocket::Core::Input::KI_ESCAPE },
	{ SDLK_SPACE,        Rocket::Core::Input::KI_SPACE },
	{ SDLK_EXCLAIM,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_QUOTEDBL,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_HASH,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_DOLLAR,       Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_AMPERSAND,    Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_QUOTE,        Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_LEFTPAREN,    Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_RIGHTPAREN,   Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_ASTERISK,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_PLUS,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_COMMA,        Rocket::Core::Input::KI_OEM_COMMA },
	{ SDLK_MINUS,        Rocket::Core::Input::KI_OEM_MINUS },
	{ SDLK_PERIOD,       Rocket::Core::Input::KI_OEM_PERIOD },
	{ SDLK_SLASH,        Rocket::Core::Input::KI_OEM_2 },
	{ SDLK_0,            Rocket::Core::Input::KI_0 },
	{ SDLK_1,            Rocket::Core::Input::KI_1 },
	{ SDLK_2,            Rocket::Core::Input::KI_2 },
	{ SDLK_3,            Rocket::Core::Input::KI_3 },
	{ SDLK_4,            Rocket::Core::Input::KI_4 },
	{ SDLK_5,            Rocket::Core::Input::KI_5 },
	{ SDLK_6,            Rocket::Core::Input::KI_6 },
	{ SDLK_7,            Rocket::Core::Input::KI_7 },
	{ SDLK_8,            Rocket::Core::Input::KI_8 },
	{ SDLK_9,            Rocket::Core::Input::KI_9 },
	{ SDLK_COLON,        Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_SEMICOLON,    Rocket::Core::Input::KI_OEM_1 },
	{ SDLK_LESS,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_EQUALS,       Rocket::Core::Input::KI_OEM_PLUS },
	{ SDLK_GREATER,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_QUESTION,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_AT,           Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_LEFTBRACKET,  Rocket::Core::Input::KI_OEM_4 },
	{ SDLK_BACKSLASH,    Rocket::Core::Input::KI_OEM_5 },
	{ SDLK_RIGHTBRACKET, Rocket::Core::Input::KI_OEM_6 },
	{ SDLK_CARET,        Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_UNDERSCORE,   Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_BACKQUOTE,    Rocket::Core::Input::KI_OEM_3 },
	{ SDLK_a,            Rocket::Core::Input::KI_A },
	{ SDLK_b,            Rocket::Core::Input::KI_B },
	{ SDLK_c,            Rocket::Core::Input::KI_C },
	{ SDLK_d,            Rocket::Core::Input::KI_D },
	{ SDLK_e,            Rocket::Core::Input::KI_E },
	{ SDLK_f,            Rocket::Core::Input::KI_F },
	{ SDLK_g,            Rocket::Core::Input::KI_G },
	{ SDLK_h,            Rocket::Core::Input::KI_H },
	{ SDLK_i,            Rocket::Core::Input::KI_I },
	{ SDLK_j,            Rocket::Core::Input::KI_J },
	{ SDLK_k,            Rocket::Core::Input::KI_K },
	{ SDLK_l,            Rocket::Core::Input::KI_L },
	{ SDLK_m,            Rocket::Core::Input::KI_M },
	{ SDLK_n,            Rocket::Core::Input::KI_N },
	{ SDLK_o,            Rocket::Core::Input::KI_O },
	{ SDLK_p,            Rocket::Core::Input::KI_P },
	{ SDLK_q,            Rocket::Core::Input::KI_Q },
	{ SDLK_r,            Rocket::Core::Input::KI_R },
	{ SDLK_s,            Rocket::Core::Input::KI_S },
	{ SDLK_t,            Rocket::Core::Input::KI_T },
	{ SDLK_u,            Rocket::Core::Input::KI_U },
	{ SDLK_v,            Rocket::Core::Input::KI_V },
	{ SDLK_w,            Rocket::Core::Input::KI_W },
	{ SDLK_x,            Rocket::Core::Input::KI_X },
	{ SDLK_y,            Rocket::Core::Input::KI_Y },
	{ SDLK_z,            Rocket::Core::Input::KI_Z },
	{ SDLK_DELETE,       Rocket::Core::Input::KI_DELETE },
	{ SDLK_WORLD_0,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_1,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_2,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_3,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_4,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_5,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_6,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_7,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_8,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_9,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_10,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_11,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_12,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_13,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_14,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_15,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_16,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_17,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_18,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_19,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_20,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_21,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_22,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_23,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_24,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_25,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_26,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_27,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_28,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_29,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_30,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_31,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_32,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_33,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_34,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_35,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_36,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_37,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_38,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_39,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_40,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_41,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_42,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_43,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_44,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_45,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_46,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_47,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_48,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_49,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_50,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_51,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_52,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_53,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_54,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_55,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_56,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_57,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_58,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_59,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_60,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_61,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_62,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_63,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_64,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_65,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_66,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_67,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_68,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_69,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_70,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_71,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_72,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_73,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_74,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_75,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_76,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_77,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_78,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_79,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_80,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_81,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_82,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_83,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_84,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_85,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_86,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_87,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_88,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_89,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_90,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_91,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_92,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_93,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_94,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_WORLD_95,     Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_KP0,          Rocket::Core::Input::KI_NUMPAD0 },
	{ SDLK_KP1,          Rocket::Core::Input::KI_NUMPAD1 },
	{ SDLK_KP2,          Rocket::Core::Input::KI_NUMPAD2 },
	{ SDLK_KP3,          Rocket::Core::Input::KI_NUMPAD3 },
	{ SDLK_KP4,          Rocket::Core::Input::KI_NUMPAD4 },
	{ SDLK_KP5,          Rocket::Core::Input::KI_NUMPAD5 },
	{ SDLK_KP6,          Rocket::Core::Input::KI_NUMPAD6 },
	{ SDLK_KP7,          Rocket::Core::Input::KI_NUMPAD7 },
	{ SDLK_KP8,          Rocket::Core::Input::KI_NUMPAD8 },
	{ SDLK_KP9,          Rocket::Core::Input::KI_NUMPAD9 },
	{ SDLK_KP_PERIOD,    Rocket::Core::Input::KI_DECIMAL },
	{ SDLK_KP_DIVIDE,    Rocket::Core::Input::KI_DIVIDE },
	{ SDLK_KP_MULTIPLY,  Rocket::Core::Input::KI_MULTIPLY },
	{ SDLK_KP_MINUS,     Rocket::Core::Input::KI_SUBTRACT },
	{ SDLK_KP_PLUS,      Rocket::Core::Input::KI_ADD },
	{ SDLK_KP_ENTER,     Rocket::Core::Input::KI_NUMPADENTER },
	{ SDLK_KP_EQUALS,    Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_UP,           Rocket::Core::Input::KI_UP },
	{ SDLK_DOWN,         Rocket::Core::Input::KI_DOWN },
	{ SDLK_RIGHT,        Rocket::Core::Input::KI_RIGHT },
	{ SDLK_LEFT,         Rocket::Core::Input::KI_LEFT },
	{ SDLK_INSERT,       Rocket::Core::Input::KI_INSERT },
	{ SDLK_HOME,         Rocket::Core::Input::KI_HOME },
	{ SDLK_END,          Rocket::Core::Input::KI_END },
	{ SDLK_PAGEUP,       Rocket::Core::Input::KI_PRIOR },
	{ SDLK_PAGEDOWN,     Rocket::Core::Input::KI_NEXT },
	{ SDLK_F1,           Rocket::Core::Input::KI_F1 },
	{ SDLK_F2,           Rocket::Core::Input::KI_F2 },
	{ SDLK_F3,           Rocket::Core::Input::KI_F3 },
	{ SDLK_F4,           Rocket::Core::Input::KI_F4 },
	{ SDLK_F5,           Rocket::Core::Input::KI_F5 },
	{ SDLK_F6,           Rocket::Core::Input::KI_F6 },
	{ SDLK_F7,           Rocket::Core::Input::KI_F7 },
	{ SDLK_F8,           Rocket::Core::Input::KI_F8 },
	{ SDLK_F9,           Rocket::Core::Input::KI_F9 },
	{ SDLK_F10,          Rocket::Core::Input::KI_F10 },
	{ SDLK_F11,          Rocket::Core::Input::KI_F11 },
	{ SDLK_F12,          Rocket::Core::Input::KI_F12 },
	{ SDLK_F13,          Rocket::Core::Input::KI_F13 },
	{ SDLK_F14,          Rocket::Core::Input::KI_F14 },
	{ SDLK_F15,          Rocket::Core::Input::KI_F15 },
	{ SDLK_NUMLOCK,      Rocket::Core::Input::KI_NUMLOCK },
	{ SDLK_CAPSLOCK,     Rocket::Core::Input::KI_CAPITAL },
	{ SDLK_SCROLLOCK,    Rocket::Core::Input::KI_SCROLL },
	{ SDLK_RSHIFT,       Rocket::Core::Input::KI_RSHIFT },
	{ SDLK_LSHIFT,       Rocket::Core::Input::KI_LSHIFT },
	{ SDLK_RCTRL,        Rocket::Core::Input::KI_RCONTROL },
	{ SDLK_LCTRL,        Rocket::Core::Input::KI_LCONTROL },
	{ SDLK_RALT,         Rocket::Core::Input::KI_RMENU },
	{ SDLK_LALT,         Rocket::Core::Input::KI_LMENU },
	{ SDLK_RMETA,        Rocket::Core::Input::KI_RMETA },
	{ SDLK_LMETA,        Rocket::Core::Input::KI_LMETA },
	{ SDLK_LSUPER,       Rocket::Core::Input::KI_LWIN },
	{ SDLK_RSUPER,       Rocket::Core::Input::KI_RWIN },
	{ SDLK_MODE,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_COMPOSE,      Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_HELP,         Rocket::Core::Input::KI_HELP },
	{ SDLK_PRINT,        Rocket::Core::Input::KI_SNAPSHOT },
	{ SDLK_SYSREQ,       Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_BREAK,        Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_MENU,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_POWER,        Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_EURO,         Rocket::Core::Input::KI_UNKNOWN },
	{ SDLK_UNDO,         Rocket::Core::Input::KI_UNKNOWN },

	{ SDLK_LAST, Rocket::Core::Input::KI_UNKNOWN }
};


class RocketSystem : public Rocket::Core::SystemInterface {
public:
	virtual float GetElapsedTime() { return float(SDL_GetTicks()) * 0.001; }
};


class RocketRender : public Rocket::Core::RenderInterface {
public:
	RocketRender(int width, int height) : Rocket::Core::RenderInterface(), m_width(width), m_height(height) {}

	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
		glPushMatrix();
		glTranslatef(translation.x, translation.y, 0);

		glVertexPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rocket::Core::Vertex), &vertices[0].colour);

		if (!texture) {
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		else {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, GLuint(texture));
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), &vertices[0].tex_coord);
		}

		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

		glPopMatrix();
	}

	virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		// XXX allocate vbo
		return Rocket::Core::CompiledGeometryHandle(0);
	}

	virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
	{
		// XXX draw vbo
	}

	virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		// XXX free vbo
	}

	virtual void EnableScissorRegion(bool enable)
	{
		if (enable)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}

	virtual void SetScissorRegion(int x, int y, int width, int height)
	{
		glScissor(x, m_height - (y + height), width, height);
	}

	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		SDL_Surface *s = IMG_Load(source.CString());
		if (!s) {
			fprintf(stderr, "RocketRender: couldn't load '%s'\n", source.CString());
			return false;
		}

		// convert if necessary. 
		SDL_PixelFormat *pixfmt = s->format;
		if (pixfmt->BytesPerPixel != rgba_pixfmt.BytesPerPixel || pixfmt->Rmask != rgba_pixfmt.Rmask || pixfmt->Gmask != rgba_pixfmt.Gmask || pixfmt->Bmask != rgba_pixfmt.Bmask)
		{
			SDL_Surface *converted = SDL_ConvertSurface(s, &rgba_pixfmt, SDL_SWSURFACE);
			SDL_FreeSurface(s);
			s = converted;
		}

		texture_dimensions.x = s->w;
		texture_dimensions.y = s->h;

		bool success = GenerateTexture(texture_handle, reinterpret_cast<Rocket::Core::byte*>(s->pixels), texture_dimensions);

		SDL_FreeSurface(s);

		return success;
	}

	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
	{
		GLuint texture_id = 0;
		glGenTextures(1, &texture_id);
		if (texture_id == 0) {
			fprintf(stderr, "RocketRender: couldn't generate texture\n");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		texture_handle = Rocket::Core::TextureHandle(texture_id);

		return true;
	}

	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
	{
		glDeleteTextures(1, reinterpret_cast<GLuint*>(&texture_handle));
	}

private:
	int m_width, m_height;
};


class RocketEventListener : public Rocket::Core::EventListener {
public:
	RocketEventListener(const std::string &eventName) : Rocket::Core::EventListener(), m_eventName(eventName) {}

	virtual void ProcessEvent(Rocket::Core::Event &e)
	{
		if (m_handler) m_handler(&e);
	}

	void SetHandler(sigc::slot<void,Rocket::Core::Event*> handler) { m_handler = handler; }

private:
	std::string m_eventName;
	sigc::slot<void,Rocket::Core::Event*> m_handler;
};


class RocketEventListenerInstancer : public Rocket::Core::EventListenerInstancer {
public:
	virtual Rocket::Core::EventListener *InstanceEventListener(const Rocket::Core::String &value)
	{
		std::string eventName(value.CString());

		std::map<std::string,RocketEventListener*>::iterator i = m_listeners.find(eventName);
		if (i != m_listeners.end())
			return (*i).second;

		RocketEventListener *listener = new RocketEventListener(eventName);
		m_listeners.insert(make_pair(eventName, listener));

		return listener;
	}

	virtual void Release() {}

private:
	std::map<std::string,RocketEventListener*> m_listeners;
};



static Rocket::Core::Input::KeyIdentifier sdlkey_to_ki[SDLK_LAST];

static bool s_initted = false;
RocketManager::RocketManager(int width, int height) : m_width(width), m_height(height), m_currentDocument(0)
{
	assert(!s_initted);
	s_initted = true;

	memset(sdlkey_to_ki, Rocket::Core::Input::KI_UNKNOWN, SDLK_LAST);

	for (const Keymap *km = keymap; km->sdl != SDLK_LAST; km++)
		sdlkey_to_ki[km->sdl] = km->rocket;

	m_rocketSystem = new RocketSystem();
	m_rocketRender = new RocketRender(m_width, m_height);

	Rocket::Core::SetSystemInterface(m_rocketSystem);
	Rocket::Core::SetRenderInterface(m_rocketRender);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	m_rocketEventListenerInstancer = new RocketEventListenerInstancer();
	Rocket::Core::Factory::RegisterEventListenerInstancer(m_rocketEventListenerInstancer);
	m_rocketEventListenerInstancer->RemoveReference();

	// XXX hook up to fontmanager or something
	Rocket::Core::FontDatabase::LoadFontFace(PIONEER_DATA_DIR "/fonts/TitilliumText22L004.otf");

	m_rocketContext = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(m_width, m_height));
}

RocketManager::~RocketManager()
{
	for (std::map<std::string,Rocket::Core::ElementDocument*>::iterator i = m_documents.begin(); i != m_documents.end(); i++)
		(*i).second->RemoveReference();

	m_rocketContext->RemoveReference();

	Rocket::Core::Shutdown();

	delete m_rocketEventListenerInstancer;

	delete m_rocketSystem;
	delete m_rocketRender;

	s_initted = false;
}

Rocket::Core::ElementDocument *RocketManager::OpenDocument(const std::string &name)
{
	std::map<std::string,Rocket::Core::ElementDocument*>::iterator i = m_documents.find(name);
	if (i != m_documents.end()) {
		// XXX check file timestamp and invalidate if changed
		if (m_currentDocument)
			m_currentDocument->Hide();
		m_currentDocument = (*i).second;

		m_currentDocument->Show();
		return m_currentDocument;
	}
	
	Rocket::Core::ElementDocument *document = m_rocketContext->LoadDocument((PIONEER_DATA_DIR "/ui/" + name + ".rml").c_str());
	if (!document) {
		fprintf(stderr, "RocketManager: couldn't load document '%s'\n", name.c_str());
		return 0;
	}

	m_documents[name] = document;
	m_currentDocument = document;

	m_currentDocument->Show();
	return m_currentDocument;
}

void RocketManager::RegisterEventHandler(const std::string &eventName, sigc::slot<void,Rocket::Core::Event*> handler)
{
	static_cast<RocketEventListener*>(m_rocketEventListenerInstancer->InstanceEventListener(Rocket::Core::String(eventName.c_str())))->SetHandler(handler);
}

void RocketManager::HandleEvent(const SDL_Event *e)
{
	SDLMod sdlmod = SDL_GetModState();
	Rocket::Core::Input::KeyModifier rmod = Rocket::Core::Input::KeyModifier(
		(sdlmod & KMOD_CTRL  ? Rocket::Core::Input::KM_CTRL      : 0) |
		(sdlmod & KMOD_SHIFT ? Rocket::Core::Input::KM_SHIFT     : 0) |
		(sdlmod & KMOD_ALT   ? Rocket::Core::Input::KM_ALT       : 0) |
		(sdlmod & KMOD_META  ? Rocket::Core::Input::KM_META      : 0) |
		(sdlmod & KMOD_CAPS  ? Rocket::Core::Input::KM_CAPSLOCK  : 0) |
		(sdlmod & KMOD_NUM   ? Rocket::Core::Input::KM_NUMLOCK   : 0)
	);
	
	switch (e->type) {
		case SDL_MOUSEMOTION:
			m_rocketContext->ProcessMouseMove(e->motion.x, e->motion.y, rmod);
			break;

		case SDL_MOUSEBUTTONDOWN:
			// XXX special handling for wheelup/wheeldown
			m_rocketContext->ProcessMouseButtonDown(
				e->button.button & SDL_BUTTON_LEFT  ? 0 :
				e->button.button & SDL_BUTTON_RIGHT ? 1 :
				                                      e->button.button, rmod);
			break;

		case SDL_MOUSEBUTTONUP:
			// XXX special handling for wheelup/wheeldown
			m_rocketContext->ProcessMouseButtonUp(
				e->button.button & SDL_BUTTON_LEFT  ? 0 :
				e->button.button & SDL_BUTTON_RIGHT ? 1 :
				                                      e->button.button, rmod);
			break;

		case SDL_KEYDOWN: {
			// XXX textinput
			Rocket::Core::Input::KeyIdentifier ki = sdlkey_to_ki[e->key.keysym.sym];
			if (ki == Rocket::Core::Input::KI_UNKNOWN)
				fprintf(stderr, "RocketInput: No keymap for SDL key %d\n", e->key.keysym.sym);
			else
				m_rocketContext->ProcessMouseButtonDown(sdlkey_to_ki[e->key.keysym.sym], rmod);
			break;
		}

		case SDL_KEYUP: {
			// XXX textinput
			Rocket::Core::Input::KeyIdentifier ki = sdlkey_to_ki[e->key.keysym.sym];
			if (ki == Rocket::Core::Input::KI_UNKNOWN)
				fprintf(stderr, "RocketInput: No keymap for SDL key %d\n", e->key.keysym.sym);
			else
				m_rocketContext->ProcessMouseButtonUp(sdlkey_to_ki[e->key.keysym.sym], rmod);
			break;
		}

		default:
			break;
	}
}

void RocketManager::Draw()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_width, m_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	m_rocketContext->Update();
	m_rocketContext->Render();
}
