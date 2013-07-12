// ========================================================================
// Licencing Infomation
// ========================================================================
// 
// Re-author: Andrew Copland (2011)
// 
// Description: After hunting around for a SIMPLE DirectDrawSurface file
//		loader I finally got fed up with overly complicated libs with
//		horrific dependencies and ripped this one mostly from the GTL.
//		Therefore the license associated (very liberal as it is) is below.
// 
//		It is not a great implementation and it is largely unfinished but
//		it handles DXT1 to 5 and is a single header which is about as
//		complex as I feel these snippets should really be.



// Game Texture Library - licensing information below.
/*
========================================================================
Licencing Infomation
========================================================================
Copyright (c)2005,2006,2007  Rob Jones  All rights reserved.
Some work carried out by Michael P. Jung, (c)2006,2007

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

========================================================================
Contact Infomation
========================================================================

Send any general questions, bug reports etc to me (Rob Jones):
  rob [at] phantom-web.co.uk
*/

#ifndef _PICODDS_H_
#define _PICODDS_H_

#include <cstddef>

typedef unsigned int    uint32_t;
typedef int             int32_t;
typedef unsigned short  uint16_t;
typedef short           int16_t;
typedef unsigned char   uint8_t;
typedef unsigned char   byte;

namespace PicoDDS
{
	enum ImgFormat
	{
		FORMAT_NONE = 0,
		FORMAT_RGB,
		FORMAT_BGR,
		FORMAT_RGBA,
		FORMAT_BGRA,
		FORMAT_ABGR,
		FORMAT_DXT1,
		FORMAT_DXT2,
		FORMAT_DXT3,
		FORMAT_DXT4,
		FORMAT_DXT5,
		FORMAT_3DC,

		//FORMAT_FLOAT16,
		//FORMAT_FLOAT32,
		//some more 3DS formats follow
		FORMAT_R32G32B32A32F,	//4 channel fp32
		FORMAT_R16G16B16A16F,	//4 channel fp16
		FORMAT_G16R16F,			//2 channel fp16
		FORMAT_G32R32F,			//2 channel fp32
		FORMAT_R16F,			//1 channel fp16
		FORMAT_R32F,			//1 channel fp16

		//additional formats for dds mainly
		//rgb formats
		FORMAT_R5G6B5,			//16bit
		FORMAT_X1R5G5B5,		//15bit
		FORMAT_A1R5G5B5,		//15bit + 1 bit alpha
		FORMAT_L8,				//luminance 
		FORMAT_A8L8,			//alpha, luminance
		FORMAT_L16,				//luminance 16bit
		FORMAT_A8,				//alpha only
		FORMAT_G16R16,			//?? normal maps? L16A16 in opengl?

		//normal map formats
		FORMAT_V8U8,			//signed format, nv_texture_shader
		FORMAT_V16U16,			//signed, nv_texture_shader
		FORMAT_Q8W8V8U8,		//signed, nv_texture_shader

		// Additional formats for PNG images
		FORMAT_RGBA16,			// RGBA 16bit (not floating point)
		FORMAT_RGB16,			// RGB 16bit (not floating point)
		FORMAT_A16,				// 16bit alpha only
		FORMAT_A16L16			// 16bit alpha and luminance
	};

	struct LoaderImgData
	{
		LoaderImgData():
			height(0),
			width(0),
			depth(0),
			colourdepth(0),
			size(0),
			numImages(0),
			numMipMaps(0),
			format(FORMAT_NONE),
			imgData(0)
		{
		};
		int32_t height;
		int32_t width;
		int32_t depth;
		int32_t colourdepth;
		int32_t size;
		int32_t numImages;
		int32_t numMipMaps;
		ImgFormat format;
		byte* imgData;
	};

	#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))

	// header should contain 'DDS ' (yes, that space is meant to be there!)
	namespace DDS
	{
		struct DDSStruct
		{
			struct pixelformatstruct
			{
				uint32_t	size;	// equals size of struct (which is part of the data file!)
				uint32_t	flags;
				uint32_t	fourCC;
				uint32_t	RGBBitCount;
				uint32_t	rBitMask;
				uint32_t	gBitMask;
				uint32_t	bBitMask;
				uint32_t	alpahbitmask;
			};
			struct ddscapsstruct
			{
				uint32_t	caps1;
				uint32_t	caps2;
				uint32_t	reserved[2];
			};

			uint32_t 	size;		// equals size of struct (which is part of the data file!)
			uint32_t	flags;
			uint32_t	height;
			uint32_t	width;
			uint32_t	sizeorpitch;
			uint32_t	depth;
			uint32_t	mipmapcount;
			uint32_t	reserved[11];
			pixelformatstruct	pixelformat;
			ddscapsstruct		ddscaps;
			uint32_t	reserved2;
		};

		// DDSStruct Flags
		const int32_t	DDSD_CAPS = 0x00000001;
		const int32_t	DDSD_HEIGHT = 0x00000002;
		const int32_t	DDSD_WIDTH = 0x00000004;
		const int32_t	DDSD_PITCH = 0x00000008;
		const int32_t	DDSD_PIXELFORMAT = 0x00001000;
		const int32_t	DDSD_MIPMAPCOUNT = 0x00020000;
		const int32_t	DDSD_LINEARSIZE = 0x00080000;
		const int32_t	DDSD_DEPTH = 0x00800000;

		// pixelformat values
		const int32_t	DDPF_ALPHAPIXELS = 0x00000001;
		const int32_t	DDPF_FOURCC = 0x00000004;
		const int32_t	DDPF_RGB = 0x00000040;

		// ddscaps
		// caps1
		const int32_t	DDSCAPS_COMPLEX = 0x00000008;
		const int32_t	DDSCAPS_TEXTURE = 0x00001000;
		const int32_t	DDSCAPS_MIPMAP = 0x00400000;
		// caps2
		const int32_t	DDSCAPS2_CUBEMAP = 0x00000200;
		const int32_t	DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
		const int32_t	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
		const int32_t	DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
		const int32_t	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
		const int32_t	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
		const int32_t	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
		const int32_t	DDSCAPS2_VOLUME = 0x00200000;
	};

	class DDSImage
	{
	public:
		DDSImage();
		DDSImage(DDSImage const &lhs);
		~DDSImage();

		size_t Read(const char* pData, const size_t dataSize);

		int GetMinDXTSize() const;

		inline int GetMipLevelSize( const unsigned int width, const unsigned int height, unsigned int depth, const ImgFormat format) const
		{
			if (!depth)
				depth=1;

			const int numPixels=width*height*depth;
			
			switch( format)
			{
			case FORMAT_L8:
			case FORMAT_A8:
				return numPixels;

			case FORMAT_R16F:
			case FORMAT_R5G6B5:
			case FORMAT_X1R5G5B5:
			case FORMAT_A1R5G5B5:
			case FORMAT_A8L8:
			case FORMAT_L16:
			case FORMAT_V8U8:
				return numPixels*2;

			case FORMAT_RGB:
			case FORMAT_BGR:
				return numPixels*3;

			case FORMAT_RGBA:
			case FORMAT_BGRA:
			case FORMAT_ABGR:
			case FORMAT_R32F:
			case FORMAT_G16R16F:
			case FORMAT_V16U16:
			case FORMAT_G16R16:
			case FORMAT_Q8W8V8U8:
				return numPixels*4;

			case FORMAT_R16G16B16A16F:
			case FORMAT_G32R32F:
				return numPixels*8;

			case FORMAT_R32G32B32A32F:
				return numPixels*16;
			
			case FORMAT_DXT1:
				return ((width+3)/4) * ((height+3)/4) * depth * 8;
			case FORMAT_DXT2:
			case FORMAT_DXT3:
			case FORMAT_DXT4:
			case FORMAT_DXT5:
			case FORMAT_3DC:
				return ((width+3)/4) * ((height+3)/4) * depth * 16;
			}
			return -1;
		}

		int CalculateStoreageSize() const;

		ImgFormat GetTextureFormat() const;

		int GetNumImages() const;

		ImgFormat GetDXTFormat() const;

#ifdef PICODDS_OPENGL
		int GetOpenGLFormat();
#endif // PICODDS_OPENGL
	protected:
	private:
		inline int GetMinSize(ImgFormat flag) const
		{
			int minsize = 1;

			switch(flag) 
			{
			case FORMAT_DXT1:
				minsize = 8;
				break;
			case FORMAT_DXT2:
			case FORMAT_DXT3:
			case FORMAT_DXT4:
			case FORMAT_DXT5:
			case FORMAT_3DC:
				minsize = 16;
				break;
			case FORMAT_NONE:
				minsize = 0;
			default:
				break;
			}
			return minsize;

		}

		inline uint16_t Read16_le(const byte* b) const
		{
			return b[0] + (b[1] << 8);
		}

		inline void Write16_le(byte* b, uint16_t value) const
		{
			b[0] = value & 0xFF;
			b[1] = value >> 8;
		}

		inline uint16_t Read16_be(const byte* b) const
		{
			return (b[0] << 8) + b[1];
		}

		inline void Write16_be(byte* b, uint16_t value) const
		{
			b[0] = value >> 8;
			b[1] = value & 0xFF;
		}

		inline uint32_t Read32_le(const byte* b) const
		{
			return Read16_le(b) + (Read16_le(b + 2) << 16);
		}

		inline uint32_t Read32_be(const byte* b) const
		{
			return (Read16_be(b) << 16) + Read16_be(b + 2);
		}

		inline uint32_t ReadDword( byte * & pData ) const
		{
			uint32_t value=Read32_le(pData);
			pData+=4;
			return value;
		}

		bool ReadHeader(const char* pDataIn, DDS::DDSStruct& header);

	public:
		LoaderImgData	imgdata_;
		bool			headerdone_;
		DDS::DDSStruct	surfacedata_;
	};

	// this is the function to call when we want to load an image
	size_t DDSLoad(const char* filename, DDSImage& dds);
}

#endif // _PICODDS_H_
