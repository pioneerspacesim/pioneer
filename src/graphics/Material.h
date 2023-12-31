// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
#include "graphics/BufferCommon.h"
#include "matrix4x4.h"

namespace Graphics {

	class Texture;
	class RendererOGL;
	class UniformBuffer;

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
	 *   with SetBufferDynamic are set *at least* once per frame; failing
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
	 * Buffer objects are simpler - if you call SetBufferDynamic, then the
	 * passed data will be copied and managed until the end of the frame.
	 * However, once SwapBuffers() is called, all dynamic data is rendered
	 * INVALID and must be re-uploaded. Failure to do so will cause the shader
	 * to read garbage data the next time you draw.
	 * If you're reading from dynamic buffer data to control loop execution in
	 * the shader, failure to update the buffer binding WILL cause a GPU hang.
	 * (There, I've saved you twenty minutes of debugging.)
	 *
	 * If your data needs to be shared between multiple materials or only needs
	 * to be updated sporadically, use SetBuffer instead; you'll get the
	 * BufferBinding object from the Renderer interface. If the buffer lives
	 * longer than a frame, you'll need to externally manage its lifetime.
	 *
	 * If you delete (or cause to be deleted by decrementing ref counts) a
	 * Texture or static Buffer object, it is your responsibility to set
	 * the Texture or Buffer reference to <nullptr>. Failure to do so will
	 * result in undefined behavior.
	 *
	 * Additionally, failure to properly set all Buffer bindings which are
	 * actively used by the shader code will likely result in undefined
	 * behavior and possibly a GPU hang or API error.
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

		virtual bool IsProgramLoaded() const = 0;

		virtual bool SetTexture(size_t hash, Texture *tex) = 0;

		// Upload the passed data and assign it to the specified buffer binding point.
		// The data will live until the end of the frame and its lifetime does not need
		// to be managed by the calling code. `buffer` is copied and may be deleted safely.
		virtual bool SetBufferDynamic(size_t hash, void *buffer, size_t size) = 0;

		// typed overload of SetBufferDynamic
		template <typename T>
		bool SetBufferDynamic(size_t hash, T *buffer) { return SetBufferDynamic(hash, static_cast<void *>(buffer), sizeof(T)); }

		// Set the given buffer object with an externally-managed uniform buffer.
		virtual bool SetBuffer(size_t hash, BufferBinding<UniformBuffer> uboBinding) = 0;

		virtual bool SetPushConstant(size_t hash, int i) = 0;
		virtual bool SetPushConstant(size_t hash, float f) = 0;
		virtual bool SetPushConstant(size_t hash, vector3f v3) = 0;
		virtual bool SetPushConstant(size_t hash, vector3f v4, float f4) = 0;
		virtual bool SetPushConstant(size_t hash, Color c) = 0;
		virtual bool SetPushConstant(size_t hash, matrix3x3f mat3) = 0;
		virtual bool SetPushConstant(size_t hash, matrix4x4f mat4) = 0;

	protected:
		MaterialDescriptor m_descriptor;
		size_t m_renderStateHash;

	private:
		friend class RendererOGL;
	};

} // namespace Graphics

#endif
