// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATERIAL_H
#define _MATERIAL_H
/*
 * Materials are used to apply an appropriate shader and other rendering parameters.
 * Users request materials from the renderer by filling a MaterialDescriptor structure,
 * and calling Renderer::CreateMaterial.
 * Users are responsible for deleting a material they have requested. This is because materials
 * are rarely shareable.
 * Material::Apply is called by renderer before drawing, and Unapply after drawing (to restore state).
 * For the OGL renderer, a Material is always accompanied by a Program.
 */
#include "Color.h"
#include "RefCounted.h"
#include "graphics/RenderState.h"
#include "matrix4x4.h"

namespace Graphics {

	class Texture;
	class RendererOGL;

	// Shorthand for unique effects
	// The other descriptor parameters may or may not have effect,
	// depends on the effect
	enum EffectType {
		EFFECT_DEFAULT,
		EFFECT_VTXCOLOR,
		EFFECT_UI,
		EFFECT_STARFIELD,
		EFFECT_PLANETRING,
		EFFECT_GEOSPHERE_TERRAIN,
		EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA,
		EFFECT_GEOSPHERE_TERRAIN_WITH_WATER,
		EFFECT_GEOSPHERE_SKY,
		EFFECT_GEOSPHERE_STAR,
		EFFECT_GASSPHERE_TERRAIN,
		EFFECT_FRESNEL_SPHERE,
		EFFECT_SHIELD,
		EFFECT_SKYBOX,
		EFFECT_SPHEREIMPOSTOR,
		EFFECT_GEN_GASGIANT_TEXTURE,
		EFFECT_BILLBOARD_ATLAS,
		EFFECT_BILLBOARD
	};

	// XXX : there must be a better place to put this
	enum MaterialQuality {
		HAS_ATMOSPHERE = 1 << 0,
		HAS_ECLIPSES = 1 << 1,
		HAS_HEAT_GRADIENT = 1 << 2,
		HAS_OCTAVES = 1 << 15
	};

	// Renderer creates a material that best matches these requirements.
	// EffectType may override some of the other flags.
	class MaterialDescriptor {
	public:
		MaterialDescriptor();
		EffectType effect;
		bool alphaTest;
		bool glowMap;
		bool ambientMap;
		bool lighting;
		bool normalMap;
		bool specularMap;
		bool usePatterns; //pattern/color system
		bool vertexColors;
		bool instanced;
		Sint32 textures;  //texture count
		Uint32 dirLights; //set by RendererOGL if lighting == true
		Uint32 quality;	  // see: Graphics::MaterialQuality

		friend bool operator==(const MaterialDescriptor &a, const MaterialDescriptor &b);
	};

	/*
	 * Materials contain all the needed information to take some triangles
	 * (or points, or lines) and get colored pixels on screen. They come with
	 * some basic state common to all materials, and any additional state is
	 * defined in <.shaderdef> files found in the data/shaders/<API> directory.
	 *
	 * Material state comes in three main types:
	 *
	 * - Push Constants are simple typed values set every time this material
	 *   is used to draw something. The underlying renderer API may have
	 *   constraints on how many push constants can be used in a single
	 *   material, so they're a good candidate for indexes into a buffer or
	 *   texture array, but not for passing large amounts of data. Expect
	 *   a worst-case maximum size of 64 bytes (1 matrix4x4f).
	 *
	 * - Textures are, like the name implies, images of any sort and size.
	 *   You can only bind a texture to a slot that has the same underlying
	 *   texture type (e.g. Texture2D, Cubemap), and it is the calling code's
	 *   responsibility to keep the texture alive until the renderer is done
	 *   using it (more on that later).
	 *
	 * - Buffers are chunks of data passed from the calling code to the shader
	 *   program running on the GPU. The structure of the buffer is determined
	 *   by agreement between the calling code and the shader; no translation
	 *   is performed by the Material system.
	 *   Buffers are allocated in increments of 256 bytes, so if you only have
	 *   one or two vectors to send to the shader, please use Push Constants
	 *   instead. Trust me, your framerate will thank you.
	 *   It is the calling code's responsibility to ensure that buffers set
	 *   with BUFFER_USAGE_DYNAMIC are set *at least* once per frame; failing
	 *   to observe this will cause the shader to be sent undefined data.
	 *
	 * Now that you know about state, let's talk about ownership.
	 *
	 * Push Constants are easy. Once you set them on a material, they stay and
	 * are used for every draw until you change them again.
	 *
	 * Once you set a texture on a material and draw, it is the calling code's
	 * responsibility to ensure that texture stays alive until the end of the
	 * frame, even if you then set another texture on the material.
	 * Most textures are acquired from the TextureCache and thus this isn't a
	 * problem. If you're doing something special that doesn't involve the
	 * TexCache, stop yourself and ask why; then remember to keep the texture
	 * alive until the next SwapBuffers().
	 *
	 * Buffer objects are simpler - if you SetBuffer(..., BUFFER_USAGE_DYNAMIC)
	 * then that buffer binding will stay there until the end of the frame.
	 * However, once SwapBuffers() is called, all dynamic buffers are rendered
	 * INVALID and must be re-set. Failure to do so will cause the shader to
	 * read garbage data the next time you draw.
	 * Setting a static buffer is easy, but once you draw using the material,
	 * you must not call SetBuffer() until the next SwapBuffers().
	 *
	 * If you delete (or cause to be deleted by decrementing ref counts) a
	 * Texture or (TDB: Buffer object), it is your responsibility to set
	 * the Texture or (TBD: Buffer) reference to <nullptr>. Failure to do so
	 * will result in undefined behavior.
	 */
	class Material : public RefCounted {
	public:
		Material();
		virtual ~Material() {}

		Color diffuse;
		Color specular;
		Color emissive;
		float shininess; //specular power 0-128

		//XXX may not be necessary. Used by newmodel to check if a material uses patterns
		const MaterialDescriptor &GetDescriptor() const { return m_descriptor; }
		//XXX may not be necessary. Used a few places to generate instanced variants,
		// could be replaced by a hash
		const RenderStateDesc &GetStateDescriptor() const { return m_stateDescriptor; }

		virtual bool IsProgramLoaded() const = 0;

		virtual bool SetTexture(size_t hash, Texture *tex) = 0;

		// TODO: do we need BufferUsage parameter?
		// TODO: expose UniformBuffer to main Renderer API, make calling code
		// responsible for owning buffer objects.
		virtual bool SetBuffer(size_t hash, void *buffer, size_t size, BufferUsage usage) = 0;
		// typed overload of SetBuffer
		template <typename T>
		bool SetBuffer(size_t hash, T *buffer, BufferUsage usage) { return SetBuffer(hash, buffer, sizeof(T), usage); }

		virtual bool SetPushConstant(size_t hash, int i) = 0;
		virtual bool SetPushConstant(size_t hash, float f) = 0;
		virtual bool SetPushConstant(size_t hash, vector3f v3) = 0;
		virtual bool SetPushConstant(size_t hash, vector3f v4, float f4) = 0;
		virtual bool SetPushConstant(size_t hash, Color c) = 0;
		virtual bool SetPushConstant(size_t hash, matrix3x3f mat3) = 0;
		virtual bool SetPushConstant(size_t hash, matrix4x4f mat4) = 0;

	protected:
		MaterialDescriptor m_descriptor;
		RenderStateDesc m_stateDescriptor;

	private:
		friend class RendererOGL;
	};

} // namespace Graphics

#endif
