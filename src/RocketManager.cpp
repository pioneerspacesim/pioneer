#include "RocketManager.h"
#include "libs.h"

#include "Rocket/Core.h"
#include "Rocket/Controls.h"

#include "Rocket/Core/SystemInterface.h"
#include "Rocket/Core/RenderInterface.h"


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
			printf("RocketRender: couldn't generate texture\n");
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


bool RocketManager::s_initted = false;

RocketManager::RocketManager(int width, int height) : m_width(width), m_height(height)
{
	assert(!s_initted);
	s_initted = true;

	m_rocketSystem = new RocketSystem();
	m_rocketRender = new RocketRender(m_width, m_height);

	Rocket::Core::SetSystemInterface(m_rocketSystem);
	Rocket::Core::SetRenderInterface(m_rocketRender);

	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();

	// XXX hook up to fontmanager or something
	Rocket::Core::FontDatabase::LoadFontFace(PIONEER_DATA_DIR "/fonts/TitilliumText22L004.otf");

	m_rocketContext = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(m_width, m_height));

	// XXX blah blah
	Rocket::Core::ElementDocument *document = m_rocketContext->LoadDocument("demo.rml");
	assert(document);
	document->Show();
	document->RemoveReference();
}

RocketManager::~RocketManager()
{
	m_rocketContext->RemoveReference();
	Rocket::Core::Shutdown();

	s_initted = false;
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
