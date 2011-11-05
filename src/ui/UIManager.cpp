#include "UIManager.h"
#include "libs.h"

#include "UIStashListElement.h"
#include "UIGaugeElement.h"

#include "Rocket/Core/SystemInterface.h"
#include "Rocket/Core/RenderInterface.h"

namespace UI {

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
	const char                         *name;
};
static const Keymap keymap[] = {
	{ SDLK_BACKSPACE,    Rocket::Core::Input::KI_BACK,        "backspace" },
	{ SDLK_TAB,          Rocket::Core::Input::KI_TAB,         "tab" },
	{ SDLK_CLEAR,        Rocket::Core::Input::KI_CLEAR,       0 },
	{ SDLK_RETURN,       Rocket::Core::Input::KI_RETURN,      "enter" },
	{ SDLK_PAUSE,        Rocket::Core::Input::KI_PAUSE,       "pause" },
	{ SDLK_ESCAPE,       Rocket::Core::Input::KI_ESCAPE,      "esc" },
	{ SDLK_SPACE,        Rocket::Core::Input::KI_SPACE,       "space" },
	{ SDLK_EXCLAIM,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_QUOTEDBL,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_HASH,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_DOLLAR,       Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_AMPERSAND,    Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_QUOTE,        Rocket::Core::Input::KI_OEM_7,       "'" },
	{ SDLK_LEFTPAREN,    Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_RIGHTPAREN,   Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_ASTERISK,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_PLUS,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_COMMA,        Rocket::Core::Input::KI_OEM_COMMA,   "," },
	{ SDLK_MINUS,        Rocket::Core::Input::KI_OEM_MINUS,   "-" },
	{ SDLK_PERIOD,       Rocket::Core::Input::KI_OEM_PERIOD,  "." },
	{ SDLK_SLASH,        Rocket::Core::Input::KI_OEM_2,       "/" },
	{ SDLK_0,            Rocket::Core::Input::KI_0,           "0" },
	{ SDLK_1,            Rocket::Core::Input::KI_1,           "1" },
	{ SDLK_2,            Rocket::Core::Input::KI_2,           "2" },
	{ SDLK_3,            Rocket::Core::Input::KI_3,           "3" },
	{ SDLK_4,            Rocket::Core::Input::KI_4,           "4" },
	{ SDLK_5,            Rocket::Core::Input::KI_5,           "5" },
	{ SDLK_6,            Rocket::Core::Input::KI_6,           "6" },
	{ SDLK_7,            Rocket::Core::Input::KI_7,           "7" },
	{ SDLK_8,            Rocket::Core::Input::KI_8,           "8" },
	{ SDLK_9,            Rocket::Core::Input::KI_9,           "9" },
	{ SDLK_COLON,        Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_SEMICOLON,    Rocket::Core::Input::KI_OEM_1,       ";" },
	{ SDLK_LESS,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_EQUALS,       Rocket::Core::Input::KI_OEM_PLUS,    "=" },
	{ SDLK_GREATER,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_QUESTION,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_AT,           Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_LEFTBRACKET,  Rocket::Core::Input::KI_OEM_4,       "[" },
	{ SDLK_BACKSLASH,    Rocket::Core::Input::KI_OEM_5,       "\\" },
	{ SDLK_RIGHTBRACKET, Rocket::Core::Input::KI_OEM_6,       "]" },
	{ SDLK_CARET,        Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_UNDERSCORE,   Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_BACKQUOTE,    Rocket::Core::Input::KI_OEM_3,       "`" },
	{ SDLK_a,            Rocket::Core::Input::KI_A,           "a" },
	{ SDLK_b,            Rocket::Core::Input::KI_B,           "b" },
	{ SDLK_c,            Rocket::Core::Input::KI_C,           "c" },
	{ SDLK_d,            Rocket::Core::Input::KI_D,           "d" },
	{ SDLK_e,            Rocket::Core::Input::KI_E,           "e" },
	{ SDLK_f,            Rocket::Core::Input::KI_F,           "f" },
	{ SDLK_g,            Rocket::Core::Input::KI_G,           "g" },
	{ SDLK_h,            Rocket::Core::Input::KI_H,           "h" },
	{ SDLK_i,            Rocket::Core::Input::KI_I,           "i" },
	{ SDLK_j,            Rocket::Core::Input::KI_J,           "j" },
	{ SDLK_k,            Rocket::Core::Input::KI_K,           "k" },
	{ SDLK_l,            Rocket::Core::Input::KI_L,           "l" },
	{ SDLK_m,            Rocket::Core::Input::KI_M,           "m" },
	{ SDLK_n,            Rocket::Core::Input::KI_N,           "n" },
	{ SDLK_o,            Rocket::Core::Input::KI_O,           "o" },
	{ SDLK_p,            Rocket::Core::Input::KI_P,           "p" },
	{ SDLK_q,            Rocket::Core::Input::KI_Q,           "q" },
	{ SDLK_r,            Rocket::Core::Input::KI_R,           "r" },
	{ SDLK_s,            Rocket::Core::Input::KI_S,           "s" },
	{ SDLK_t,            Rocket::Core::Input::KI_T,           "t" },
	{ SDLK_u,            Rocket::Core::Input::KI_U,           "u" },
	{ SDLK_v,            Rocket::Core::Input::KI_V,           "v" },
	{ SDLK_w,            Rocket::Core::Input::KI_W,           "w" },
	{ SDLK_x,            Rocket::Core::Input::KI_X,           "x" },
	{ SDLK_y,            Rocket::Core::Input::KI_Y,           "y" },
	{ SDLK_z,            Rocket::Core::Input::KI_Z,           "z" },
	{ SDLK_DELETE,       Rocket::Core::Input::KI_DELETE,      "delete" },
	{ SDLK_WORLD_0,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_1,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_2,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_3,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_4,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_5,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_6,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_7,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_8,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_9,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_10,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_11,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_12,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_13,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_14,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_15,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_16,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_17,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_18,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_19,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_20,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_21,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_22,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_23,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_24,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_25,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_26,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_27,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_28,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_29,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_30,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_31,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_32,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_33,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_34,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_35,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_36,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_37,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_38,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_39,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_40,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_41,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_42,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_43,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_44,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_45,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_46,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_47,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_48,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_49,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_50,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_51,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_52,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_53,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_54,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_55,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_56,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_57,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_58,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_59,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_60,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_61,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_62,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_63,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_64,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_65,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_66,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_67,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_68,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_69,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_70,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_71,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_72,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_73,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_74,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_75,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_76,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_77,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_78,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_79,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_80,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_81,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_82,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_83,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_84,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_85,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_86,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_87,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_88,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_89,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_90,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_91,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_92,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_93,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_94,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_WORLD_95,     Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_KP0,          Rocket::Core::Input::KI_NUMPAD0,     "kp0" },
	{ SDLK_KP1,          Rocket::Core::Input::KI_NUMPAD1,     "kp1" },
	{ SDLK_KP2,          Rocket::Core::Input::KI_NUMPAD2,     "kp2" },
	{ SDLK_KP3,          Rocket::Core::Input::KI_NUMPAD3,     "kp3" },
	{ SDLK_KP4,          Rocket::Core::Input::KI_NUMPAD4,     "kp4" },
	{ SDLK_KP5,          Rocket::Core::Input::KI_NUMPAD5,     "kp5" },
	{ SDLK_KP6,          Rocket::Core::Input::KI_NUMPAD6,     "kp6" },
	{ SDLK_KP7,          Rocket::Core::Input::KI_NUMPAD7,     "kp7" },
	{ SDLK_KP8,          Rocket::Core::Input::KI_NUMPAD8,     "kp8" },
	{ SDLK_KP9,          Rocket::Core::Input::KI_NUMPAD9,     "kp9" },
	{ SDLK_KP_PERIOD,    Rocket::Core::Input::KI_DECIMAL,     "kp." },
	{ SDLK_KP_DIVIDE,    Rocket::Core::Input::KI_DIVIDE,      "kp/" },
	{ SDLK_KP_MULTIPLY,  Rocket::Core::Input::KI_MULTIPLY,    "kp*" },
	{ SDLK_KP_MINUS,     Rocket::Core::Input::KI_SUBTRACT,    "kp-" },
	{ SDLK_KP_PLUS,      Rocket::Core::Input::KI_ADD,         "kp+" },
	{ SDLK_KP_ENTER,     Rocket::Core::Input::KI_NUMPADENTER, "kpenter" },
	{ SDLK_KP_EQUALS,    Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_UP,           Rocket::Core::Input::KI_UP,          "up" },
	{ SDLK_DOWN,         Rocket::Core::Input::KI_DOWN,        "down" },
	{ SDLK_RIGHT,        Rocket::Core::Input::KI_RIGHT,       "right" },
	{ SDLK_LEFT,         Rocket::Core::Input::KI_LEFT,        "left" },
	{ SDLK_INSERT,       Rocket::Core::Input::KI_INSERT,      "insert" },
	{ SDLK_HOME,         Rocket::Core::Input::KI_HOME,        "home" },
	{ SDLK_END,          Rocket::Core::Input::KI_END,         "end" },
	{ SDLK_PAGEUP,       Rocket::Core::Input::KI_PRIOR,       "pageup" },
	{ SDLK_PAGEDOWN,     Rocket::Core::Input::KI_NEXT,        "pagedown" },
	{ SDLK_F1,           Rocket::Core::Input::KI_F1,          "f1" },
	{ SDLK_F2,           Rocket::Core::Input::KI_F2,          "f2" },
	{ SDLK_F3,           Rocket::Core::Input::KI_F3,          "f3" },
	{ SDLK_F4,           Rocket::Core::Input::KI_F4,          "f4" },
	{ SDLK_F5,           Rocket::Core::Input::KI_F5,          "f5" },
	{ SDLK_F6,           Rocket::Core::Input::KI_F6,          "f6" },
	{ SDLK_F7,           Rocket::Core::Input::KI_F7,          "f7" },
	{ SDLK_F8,           Rocket::Core::Input::KI_F8,          "f8" },
	{ SDLK_F9,           Rocket::Core::Input::KI_F9,          "f9" },
	{ SDLK_F10,          Rocket::Core::Input::KI_F10,         "f10" },
	{ SDLK_F11,          Rocket::Core::Input::KI_F11,         "f11" },
	{ SDLK_F12,          Rocket::Core::Input::KI_F12,         "f12" },
	{ SDLK_F13,          Rocket::Core::Input::KI_F13,         "f13" },
	{ SDLK_F14,          Rocket::Core::Input::KI_F14,         "f14" },
	{ SDLK_F15,          Rocket::Core::Input::KI_F15,         "f15" },
	{ SDLK_NUMLOCK,      Rocket::Core::Input::KI_NUMLOCK,     0 },
	{ SDLK_CAPSLOCK,     Rocket::Core::Input::KI_CAPITAL,     0 },
	{ SDLK_SCROLLOCK,    Rocket::Core::Input::KI_SCROLL,      0 },
	{ SDLK_RSHIFT,       Rocket::Core::Input::KI_RSHIFT,      0 },
	{ SDLK_LSHIFT,       Rocket::Core::Input::KI_LSHIFT,      0 },
	{ SDLK_RCTRL,        Rocket::Core::Input::KI_RCONTROL,    0 },
	{ SDLK_LCTRL,        Rocket::Core::Input::KI_LCONTROL,    0 },
	{ SDLK_RALT,         Rocket::Core::Input::KI_RMENU,       0 },
	{ SDLK_LALT,         Rocket::Core::Input::KI_LMENU,       0 },
	{ SDLK_RMETA,        Rocket::Core::Input::KI_RMETA,       0 },
	{ SDLK_LMETA,        Rocket::Core::Input::KI_LMETA,       0 },
	{ SDLK_LSUPER,       Rocket::Core::Input::KI_LWIN,        0 },
	{ SDLK_RSUPER,       Rocket::Core::Input::KI_RWIN,        0 },
	{ SDLK_MODE,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_COMPOSE,      Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_HELP,         Rocket::Core::Input::KI_HELP,        "help" },
	{ SDLK_PRINT,        Rocket::Core::Input::KI_SNAPSHOT,    "printscreen" },
	{ SDLK_SYSREQ,       Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_BREAK,        Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_MENU,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_POWER,        Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_EURO,         Rocket::Core::Input::KI_UNKNOWN,     0 },
	{ SDLK_UNDO,         Rocket::Core::Input::KI_UNKNOWN,     0 },

	{ SDLK_LAST, Rocket::Core::Input::KI_UNKNOWN, 0 }
};


class RocketSystemInterface : public Rocket::Core::SystemInterface {
public:
	virtual float GetElapsedTime() { return float(SDL_GetTicks()) * 0.001; }
};


class RocketRenderInterface : public Rocket::Core::RenderInterface {
public:
	RocketRenderInterface(int width, int height) : Rocket::Core::RenderInterface(), m_width(width), m_height(height) {}

	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
		glPushMatrix();
		glTranslatef(translation.x, translation.y, 0);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), &vertices[0].position);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rocket::Core::Vertex), &vertices[0].colour);

		if (texture) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, GLuint(texture));
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), &vertices[0].tex_coord);
		}

		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

		if (texture) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();
	}

	struct BufferOffset {
		void *vertexOffset;
		void *colourOffset;
		void *texCoordOffset;
	};
	static const BufferOffset s_bufferOffset;

	struct VBO {
		GLuint id;
		GLuint indexId;
		int numIndices;
		GLuint texture;
	};

	// XXX this creates a new vbo for every piece of compiled geometry, which
	// means at least one per glyph. this needs optimising
	virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		VBO *vbo = new VBO;
		vbo->numIndices = num_indices;
		vbo->texture = GLuint(texture);

		glGenBuffersARB(1, &vbo->id);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->id);

		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Rocket::Core::Vertex) * num_vertices, 0, GL_STATIC_DRAW);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(Rocket::Core::Vertex) * num_vertices, vertices);

		if (0 != vbo->texture)
			glTexCoordPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), s_bufferOffset.texCoordOffset);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rocket::Core::Vertex), s_bufferOffset.colourOffset);
		glVertexPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), s_bufferOffset.vertexOffset);

		glGenBuffersARB(1, &vbo->indexId);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->indexId);

		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(int) * vbo->numIndices, indices, GL_STATIC_DRAW);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		return Rocket::Core::CompiledGeometryHandle(vbo);
	}

	virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
	{
		VBO *vbo = reinterpret_cast<VBO*>(geometry);

		glPushMatrix();
		glTranslatef(translation.x, translation.y, 0);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->id);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo->indexId);

		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		if (0 != vbo->texture) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), s_bufferOffset.texCoordOffset);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, vbo->texture);
		}

		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Rocket::Core::Vertex), s_bufferOffset.colourOffset);
		glVertexPointer(2, GL_FLOAT, sizeof(Rocket::Core::Vertex), s_bufferOffset.vertexOffset);

		glDrawElements(GL_TRIANGLES, vbo->numIndices, GL_UNSIGNED_INT, s_bufferOffset.vertexOffset);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		if (0 != vbo->texture) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		glPopMatrix();
	}

	virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		VBO *vbo = reinterpret_cast<VBO*>(geometry);

		glDeleteBuffersARB(1, &vbo->id);
		glDeleteBuffersARB(1, &vbo->indexId);

		delete vbo;
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
			fprintf(stderr, "UIRocketRenderInterface: couldn't load '%s'\n", source.CString());
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
			fprintf(stderr, "UIRocketRenderInterface: couldn't generate texture\n");
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		texture_handle = Rocket::Core::TextureHandle(texture_id);

		glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
	{
		glDeleteTextures(1, reinterpret_cast<GLuint*>(&texture_handle));
	}

private:
	int m_width, m_height;
};

const RocketRenderInterface::BufferOffset RocketRenderInterface::s_bufferOffset = {
	reinterpret_cast<void*>(offsetof(Rocket::Core::Vertex, position)),
	reinterpret_cast<void*>(offsetof(Rocket::Core::Vertex, colour)),
	reinterpret_cast<void*>(offsetof(Rocket::Core::Vertex, tex_coord))
};

static Rocket::Core::Input::KeyIdentifier sdlkey_to_ki[SDLK_LAST];
static std::map<std::string,Rocket::Core::Input::KeyIdentifier> name_to_ki;


class EventListenerInstancer : public Rocket::Core::EventListenerInstancer {
public:
	EventListenerInstancer(Manager *manager) : Rocket::Core::EventListenerInstancer(), m_manager(manager) {}

	virtual Rocket::Core::EventListener *InstanceEventListener(const Rocket::Core::String &value, Rocket::Core::Element *element)
	{
		std::string eventName(value.CString());

		Screen *screen = m_manager->GetCurrentScreen();
		if (!screen) return 0;

		EventListener *listener = screen->GetEventListener(eventName);

		Rocket::Core::String shortcut = element->GetAttribute<Rocket::Core::String>("shortcut", "");
		if (shortcut.Length() > 0) {
			Rocket::Core::Input::KeyIdentifier ki = Rocket::Core::Input::KI_UNKNOWN;
			Uint32 km = 0;

			int start = 0;
			while (start < int(shortcut.Length())) {
				int end = shortcut.Find(" ", start);
				if (end < 0)
					end = shortcut.Length();
				else
					while (end+1 < int(shortcut.Length()) && shortcut[end+1] == ' ') end++;
				
				std::string token(shortcut.Substring(start, end-start).ToLower().CString());
				start = end+1;

				if (token == "ctrl")
					km |= Rocket::Core::Input::KM_CTRL;

				else if (token == "shift")
					km |= Rocket::Core::Input::KM_SHIFT;

				else if (token == "alt")
					km |= Rocket::Core::Input::KM_ALT;

				else if (token == "meta")
					km |= Rocket::Core::Input::KM_META;

				else
					ki = name_to_ki[token];
			}

			if (ki != Rocket::Core::Input::KI_UNKNOWN)
				screen->RegisterKeyboardShortcut(ki, Rocket::Core::Input::KeyModifier(km), eventName);
			else
				fprintf(stderr, "UIInput: Couldn't convert shortcut spec '%s'\n", shortcut.CString());
		}

		return listener;
	}

	virtual void Release() {}

private:
	Manager *m_manager;
};


Screen::~Screen()
{
	for (std::map<std::string,EventListener*>::iterator i = m_eventListeners.begin(); i != m_eventListeners.end(); ++i)
		delete (*i).second;
}

void Screen::SetDocument(Rocket::Core::ElementDocument *document)
{
	assert(!m_document);
	m_document = document;
}

void Screen::RegisterKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier, const std::string &eventName)
{
	ShortcutPair pair = { key, modifier };
	m_shortcuts[pair] = eventName;
}

void Screen::ProcessKeyboardShortcut(Rocket::Core::Input::KeyIdentifier key, Rocket::Core::Input::KeyModifier modifier)
{
	ShortcutPair pair = { key, modifier };
	std::map<ShortcutPair,std::string>::iterator i = m_shortcuts.find(pair);
	if (i == m_shortcuts.end())
		return;

	std::map<std::string,EventListener*>::iterator j = m_eventListeners.find((*i).second);
	assert((*j).second);

	(*j).second->CallHandler();
}

EventListener *Screen::GetEventListener(const std::string &eventName)
{
	std::map<std::string,EventListener*>::iterator i = m_eventListeners.find(eventName);
	if (i != m_eventListeners.end())
		return (*i).second;

	EventListener *listener = new EventListener(eventName);
	m_eventListeners.insert(make_pair(eventName, listener));

	return listener;
}

void Screen::ShowTooltip(Rocket::Core::Element *sourceElement)
{
	if (m_tooltipActive) return;

	Rocket::Core::String text = sourceElement->GetAttribute<Rocket::Core::String>("tooltip", "");
	if (text.Length() == 0) {
		m_tooltipActive = true;
		return;
	}

	m_tooltipElement = m_document->CreateElement("tooltip");

	Rocket::Core::ElementText *textNode = m_document->CreateTextNode(text);
	m_tooltipElement->AppendChild(textNode);

	m_tooltipElement->SetProperty("left", Rocket::Core::Property(sourceElement->GetAbsoluteLeft()-m_document->GetAbsoluteLeft()+10.0f, Rocket::Core::Property::NUMBER));
	m_tooltipElement->SetProperty("top", Rocket::Core::Property(sourceElement->GetAbsoluteTop()-m_document->GetAbsoluteTop()+10.0f, Rocket::Core::Property::NUMBER));

	m_document->AppendChild(m_tooltipElement);

	m_tooltipElement->RemoveReference();
	textNode->RemoveReference();

	m_tooltipActive = true;
}

void Screen::ClearTooltip()
{
	if (m_tooltipElement) m_document->RemoveChild(m_tooltipElement);
	m_tooltipActive = false;
	m_tooltipElement = 0;
}


static bool s_initted = false;
Manager::Manager(int width, int height) :
	m_width(width),
	m_height(height),
	m_backgroundDocument(0),
	m_currentScreen(0),
	m_currentKey(Rocket::Core::Input::KI_UNKNOWN),
	m_currentModifier(Rocket::Core::Input::KeyModifier(0)),
	m_tooltipDelayStartTick(0),
	m_tooltipSourceElement(0)
{
	assert(!s_initted);
	s_initted = true;

	memset(sdlkey_to_ki, Rocket::Core::Input::KI_UNKNOWN, SDLK_LAST);

	for (const Keymap *km = keymap; km->sdl != SDLK_LAST; km++) {
		sdlkey_to_ki[km->sdl] = km->rocket;
		if (km->name) name_to_ki[km->name] = km->rocket;
	}

	m_rocketSystemInterface = new RocketSystemInterface();
	m_rocketRenderInterface = new RocketRenderInterface(m_width, m_height);

	Rocket::Core::SetSystemInterface(m_rocketSystemInterface);
	Rocket::Core::SetRenderInterface(m_rocketRenderInterface);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	StashListElement::Register();
	GaugeElement::Register();

	m_eventListenerInstancer = new EventListenerInstancer(this);
	Rocket::Core::Factory::RegisterEventListenerInstancer(m_eventListenerInstancer);
	m_eventListenerInstancer->RemoveReference();

	// XXX hook up to fontmanager or something
	Rocket::Core::FontDatabase::LoadFontFace(PIONEER_DATA_DIR "/fonts/TitilliumText22L004.otf");

	m_backgroundContext = Rocket::Core::CreateContext("background", Rocket::Core::Vector2i(m_width, m_height));

	m_mainContext = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(m_width, m_height));
	m_mainContext->AddEventListener("keyup", this, true);
	m_mainContext->AddEventListener("keydown", this, true);
	m_mainContext->AddEventListener("mousemove", this, false);
}

Manager::~Manager()
{
	for (std::map<std::string,Screen*>::iterator i = m_screens.begin(); i != m_screens.end(); ++i)
		(*i).second->GetDocument()->RemoveReference();

	m_mainContext->RemoveReference();
	m_backgroundContext->RemoveReference();

	Rocket::Core::Shutdown();

	for (std::map<std::string,Screen*>::iterator i = m_screens.begin(); i != m_screens.end(); ++i)
		delete (*i).second;

	// XXX no way to clear the event listener instancer registered with
	// rocket, so it will retain this pointer. future document builds will
	// crash
	delete m_eventListenerInstancer;

	delete m_rocketSystemInterface;
	delete m_rocketRenderInterface;

	s_initted = false;
}

Screen *Manager::OpenScreen(const std::string &name)
{
	if (m_currentScreen)
		m_currentScreen->GetDocument()->Hide();

	std::map<std::string,Screen*>::iterator i = m_screens.find(name);
	if (i == m_screens.end()) {
		m_currentScreen = new Screen();

		Rocket::Core::ElementDocument *document = m_mainContext->LoadDocument((PIONEER_DATA_DIR "/ui/" + name + ".rml").c_str());
		if (!document) {
			fprintf(stderr, "UIManager: couldn't load document '%s'\n", name.c_str());
			delete m_currentScreen;
			m_currentScreen = 0;
			return 0;
		}

		m_currentScreen->SetDocument(document);

		m_screens[name] = m_currentScreen;
	}
	else {
		// XXX check file timestamp and invalidate if changed
		m_currentScreen = (*i).second;
	}

	m_currentScreen->ClearTooltip();
	m_tooltipDelayStartTick = SDL_GetTicks();
	m_tooltipSourceElement = 0;

	Update(m_currentScreen->GetDocument(), true);

	m_currentScreen->GetDocument()->Show();

	return m_currentScreen;
}

// XXX too much overlap in code and concept with the screen stuff, but its the
// simplest way to prototype this
void Manager::OpenBackground(const std::string &name)
{
	if (m_backgroundDocument)
		m_backgroundDocument->Hide();
	
	std::map<std::string,Rocket::Core::ElementDocument*>::iterator i = m_backgroundDocuments.find(name);
	if (i == m_backgroundDocuments.end()) {
		m_backgroundDocument = m_backgroundContext->LoadDocument((PIONEER_DATA_DIR "/ui/" + name + ".rml").c_str());
		if (!m_backgroundDocument) {
			fprintf(stderr, "UIManager: couldn't load document '%s'\n", name.c_str());
			return;
		}
		m_backgroundDocuments[name] = m_backgroundDocument;
	}
	else
		m_backgroundDocument = (*i).second;

	Update(m_backgroundDocument, true);

	m_backgroundDocument->Show();
}

void Manager::ProcessEvent(Rocket::Core::Event &e)
{
	if (!m_currentScreen) return;

	if (e.GetType() == "mousemove" && e.GetTargetElement() != m_tooltipSourceElement && e.GetTargetElement()->GetTagName() != "tooltip") {
		m_currentScreen->ClearTooltip();
		m_tooltipDelayStartTick = SDL_GetTicks();
		m_tooltipSourceElement = e.GetTargetElement();
		return;
	}

	Rocket::Core::Input::KeyIdentifier key = Rocket::Core::Input::KeyIdentifier(e.GetParameter<int>("key_identifier", int(Rocket::Core::Input::KI_UNKNOWN)));

	Rocket::Core::Input::KeyModifier modifier = Rocket::Core::Input::KeyModifier(
		(e.GetParameter<bool>("ctrl_key", false)  ? Rocket::Core::Input::KM_CTRL  : 0) |
		(e.GetParameter<bool>("shift_key", false) ? Rocket::Core::Input::KM_SHIFT : 0) |
		(e.GetParameter<bool>("altl_key", false)  ? Rocket::Core::Input::KM_ALT   : 0) |
		(e.GetParameter<bool>("meta_key", false)  ? Rocket::Core::Input::KM_META  : 0)
	);
	
	if (e.GetType() == "keydown") {
		m_currentKey = key;
		m_currentModifier = modifier;
	}
	
	else if (m_currentKey == key && m_currentModifier == modifier)
		m_currentScreen->ProcessKeyboardShortcut(key, modifier);
}

void Manager::HandleEvent(const SDL_Event *e)
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
			m_mainContext->ProcessMouseMove(e->motion.x, e->motion.y, rmod);
			break;

		case SDL_MOUSEBUTTONDOWN:
			// XXX special handling for wheelup/wheeldown
			m_mainContext->ProcessMouseButtonDown(
				e->button.button & SDL_BUTTON_LEFT  ? 0 :
				e->button.button & SDL_BUTTON_RIGHT ? 1 :
				                                      e->button.button, rmod);
			break;

		case SDL_MOUSEBUTTONUP:
			// XXX special handling for wheelup/wheeldown
			m_mainContext->ProcessMouseButtonUp(
				e->button.button & SDL_BUTTON_LEFT  ? 0 :
				e->button.button & SDL_BUTTON_RIGHT ? 1 :
				                                      e->button.button, rmod);
			break;

		case SDL_KEYDOWN: {
			// XXX textinput
			Rocket::Core::Input::KeyIdentifier ki = sdlkey_to_ki[e->key.keysym.sym];
			if (ki == Rocket::Core::Input::KI_UNKNOWN)
				fprintf(stderr, "UIInput: No keymap for SDL key %d\n", e->key.keysym.sym);
			else
				m_mainContext->ProcessKeyDown(sdlkey_to_ki[e->key.keysym.sym], rmod);
			break;
		}

		case SDL_KEYUP: {
			// XXX textinput
			Rocket::Core::Input::KeyIdentifier ki = sdlkey_to_ki[e->key.keysym.sym];
			if (ki == Rocket::Core::Input::KI_UNKNOWN)
				fprintf(stderr, "UIInput: No keymap for SDL key %d\n", e->key.keysym.sym);
			else
				m_mainContext->ProcessKeyUp(sdlkey_to_ki[e->key.keysym.sym], rmod);
			break;
		}

		default:
			break;
	}
}

void Manager::Draw()
{
	if (!m_currentScreen && !m_backgroundDocument)
		return;

	if (m_backgroundDocument) {
		Update(m_backgroundDocument);
		m_backgroundContext->Update();
	}

	if (m_currentScreen) {
		Update(m_currentScreen->GetDocument());
		m_mainContext->Update();

		if (m_tooltipSourceElement && m_tooltipDelayStartTick + 2000 <= SDL_GetTicks())   // 2s mouse stop for tooltips
			m_currentScreen->ShowTooltip(m_tooltipSourceElement);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_width, m_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	if (m_backgroundDocument)
		m_backgroundContext->Render();

	if (m_currentScreen)
		m_mainContext->Render();
}

}
