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

#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>

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
			imgData(nullptr)
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
		DDSImage()
		{
			memset(&imgdata_,0,sizeof(imgdata_));
			memset(&surfacedata_,0,sizeof(surfacedata_));
		};

		DDSImage(DDSImage const &lhs) : imgdata_(lhs.imgdata_), surfacedata_(lhs.surfacedata_)
		{

		};

		~DDSImage()
		{

		}

		size_t read(FILE* pFile)
		{
			// Read in header and decode
			if (!ReadHeader(pFile, surfacedata_))
				return -1;

			if (surfacedata_.mipmapcount==0)
				surfacedata_.mipmapcount=1;

			imgdata_.height = surfacedata_.height;
			imgdata_.width = surfacedata_.width;

			if(surfacedata_.flags & DDS::DDSD_DEPTH)
				imgdata_.depth = surfacedata_.depth;
			else
				imgdata_.depth = 0;	// no depth to these images

			imgdata_.colourdepth = surfacedata_.pixelformat.RGBBitCount;
			imgdata_.numMipMaps = surfacedata_.mipmapcount;
			imgdata_.format = getTextureFormat();
			imgdata_.numImages = getNumImages();
			imgdata_.size = calculateStoreageSize();
			if(0 >= imgdata_.size)
				return -1;
			
			if(-1 == imgdata_.format)
				return -1;

			fseek(pFile, 0, SEEK_END); // seek to end of file
			size_t size = ftell(pFile); // get current file pointer
			const long headerSize=128;
			const size_t DDSStructSize = sizeof(DDS::DDSStruct)+4;
			fseek(pFile, 0, SEEK_SET); // seek back to beginning of file
			fseek(pFile, headerSize, SEEK_SET); // seek back to beginning of file
			// proceed with allocating memory and reading the file
			imgdata_.imgData = new byte[imgdata_.size];

			// Read in remaining data
			size_t amtRead = fread(imgdata_.imgData, sizeof(byte), size-headerSize, pFile);
			return amtRead;
		}

		int getMinDXTSize()
		{
			return getMinSize(getDXTFormat());
		}

		inline int GetMipLevelSize( unsigned int width, unsigned int height, unsigned int depth, ImgFormat format)
		{
			if (!depth)
				depth=1;

			int numPixels=width*height*depth;
			
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

		int calculateStoreageSize()
		{
			int size = 0;
			for(int i = 0; i < imgdata_.numImages; ++i)
			{
				int width=imgdata_.width;
				int height=imgdata_.height;
				int depth=imgdata_.depth;

				for (int m=0; m<imgdata_.numMipMaps; ++m)
				{
					size+=GetMipLevelSize(width, height, depth, imgdata_.format);
					width = std::max(width>>1, 1);
					height = std::max(height>>1, 1);
					depth = std::max(depth>>1, 1);
				}
			}

			return size;
		}

		ImgFormat getTextureFormat()
		{
			ImgFormat format = FORMAT_NONE;

			if(surfacedata_.pixelformat.flags & DDS::DDPF_FOURCC)
			{
				format = getDXTFormat();
			} 
			else if(surfacedata_.pixelformat.flags & DDS::DDPF_RGB)
			{
				if(surfacedata_.pixelformat.flags & DDS::DDPF_ALPHAPIXELS)
				{
					if (0xff == surfacedata_.pixelformat.bBitMask &&
						0xff00 == surfacedata_.pixelformat.gBitMask &&
						0xff0000 == surfacedata_.pixelformat.rBitMask &&
						0xff000000 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata_.pixelformat.rBitMask &&
								0xff00 == surfacedata_.pixelformat.gBitMask &&
								0xff0000 == surfacedata_.pixelformat.bBitMask &&
								0xff000000 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_RGBA;
					} else if (	0xff == surfacedata_.pixelformat.alpahbitmask &&
								0xff00 == surfacedata_.pixelformat.bBitMask &&
								0xff0000 == surfacedata_.pixelformat.gBitMask &&
								0xff000000 == surfacedata_.pixelformat.rBitMask)
					{
						format = FORMAT_ABGR;
					} else if (	0x8000 == surfacedata_.pixelformat.alpahbitmask &&
								0x1F == surfacedata_.pixelformat.bBitMask &&
								0x3E0 == surfacedata_.pixelformat.gBitMask &&
								0x7C00 == surfacedata_.pixelformat.rBitMask)
					{
						format = FORMAT_A1R5G5B5;
					}
				}
				else
				{
					if (0xff == surfacedata_.pixelformat.bBitMask &&
						0xff00 == surfacedata_.pixelformat.gBitMask &&
						0xff0000 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata_.pixelformat.rBitMask &&
								0xff00 == surfacedata_.pixelformat.gBitMask &&
								0xff0000 == surfacedata_.pixelformat.bBitMask )
					{
						format = FORMAT_RGBA;
					} else if (	0xffFF == surfacedata_.pixelformat.rBitMask &&
								0xffFF0000 == surfacedata_.pixelformat.gBitMask &&
								0x00 == surfacedata_.pixelformat.bBitMask &&
								0x00 == surfacedata_.pixelformat.alpahbitmask)
					{
						format = FORMAT_G16R16;
					} else if (	0x1F == surfacedata_.pixelformat.bBitMask &&
								0x3E0 == surfacedata_.pixelformat.gBitMask &&
								0x7C00 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_X1R5G5B5;
					} else if (	0x1F == surfacedata_.pixelformat.bBitMask &&
								0x7E0 == surfacedata_.pixelformat.gBitMask &&
								0xF800 == surfacedata_.pixelformat.rBitMask )
					{
						format = FORMAT_R5G6B5;
					}
				}
			} else
			{
				if (0xFF==surfacedata_.pixelformat.rBitMask &&
					0x0==surfacedata_.pixelformat.gBitMask &&
					0x0==surfacedata_.pixelformat.bBitMask &&
					0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_L8;
				} else if (	0xFFFF==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_L16;
				} else if (	0x0==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0xFF==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0x0==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0xFF00==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8L8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0xFF00==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_V8U8;
				} else if (	0xFF==surfacedata_.pixelformat.rBitMask &&
							0xFF00==surfacedata_.pixelformat.gBitMask &&
							0xFF0000==surfacedata_.pixelformat.bBitMask &&
							0xFF000000==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_Q8W8V8U8;
				} else if (	0xFFFF==surfacedata_.pixelformat.rBitMask &&
							0xFFFF0000==surfacedata_.pixelformat.gBitMask &&
							0x0==surfacedata_.pixelformat.bBitMask &&
							0x0==surfacedata_.pixelformat.alpahbitmask)
				{
					format = FORMAT_V16U16;
				}
			}
			return format;
		}

		int getNumImages()
		{
			if(!(surfacedata_.ddscaps.caps2 & DDS::DDSCAPS2_CUBEMAP))
				return 1;

			// We are a cube map, so work out how many sides we have
			uint32_t mask = DDS::DDSCAPS2_CUBEMAP_POSITIVEX;
			int count = 0;
			for(int n = 0; n < 6; ++n)
			{
				if(surfacedata_.ddscaps.caps2 & mask)
					++count;
				mask *= 2;	// move to next face
			}
			return count;		
		}

		ImgFormat getDXTFormat()
		{
			ImgFormat format = FORMAT_NONE;
			switch(surfacedata_.pixelformat.fourCC) 
			{
			case FOURCC('D','X','T','1'):
				format = FORMAT_DXT1;
				break;
			case FOURCC('D','X','T','2'):
				format = FORMAT_DXT2;
				break;
			case FOURCC('D','X','T','3'):
				format = FORMAT_DXT3;
				break;
			case FOURCC('D','X','T','4'):
				format = FORMAT_DXT4;
				break;
			case FOURCC('D','X','T','5'):
				format = FORMAT_DXT5;
				break;
			case FOURCC('A','T','I','2'):
				format = FORMAT_3DC;
				break;
			case 0x74:
				format=FORMAT_R32G32B32A32F;
				break;
			case 0x71:
				format=FORMAT_R16G16B16A16F;
				break;
			case 0x70:
				format=FORMAT_G16R16F;
				break;
			case 0x73:
				format=FORMAT_G32R32F;
				break;
			case 0x6F:
				format=FORMAT_R16F;
				break;
			case 0x72:
				format=FORMAT_R32F;
				break;
			default:
				break;
			}
			return format;
		}

#ifdef PICODDS_OPENGL
		int getOpenGLFormat()
		{
			int format = -1;
			switch(surfacedata_.pixelformat.fourCC) 
			{
			case FOURCC('D','X','T','1'):
				if( surfacedata_.pixelformat.flags & DDS::DDPF_ALPHAPIXELS ) {
					format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;	// dxt1 with 1bit alpha
				} else {
					format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				}
				break;
			case FOURCC('D','X','T','2'):
				format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				break;
			case FOURCC('D','X','T','3'):
			case FOURCC('D','X','T','4'):
				format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				break;
			case FOURCC('D','X','T','5'):
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				break;
			case FOURCC('A','T','I','2'):
				//format = GL_COMPRESSED_RGB_3DC_ATI;
				//break;
			default:
				assert(false && "Do not currently support un-compressed files or ATI2."); 
				break;
			}
			return format;
		}
#endif // PICODDS_OPENGL
	protected:
	private:
		inline int getMinSize(ImgFormat flag)
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

		inline uint16_t read16_le(const byte* b) 
		{
			return b[0] + (b[1] << 8);
		}

		inline void write16_le(byte* b, uint16_t value) 
		{
			b[0] = value & 0xFF;
			b[1] = value >> 8;
		}

		inline uint16_t read16_be(const byte* b) 
		{
			return (b[0] << 8) + b[1];
		}

		inline void write16_be(byte* b, uint16_t value) 
		{
			b[0] = value >> 8;
			b[1] = value & 0xFF;
		}

		inline uint32_t read32_le(const byte* b) 
		{
			return read16_le(b) + (read16_le(b + 2) << 16);
		}

		inline uint32_t read32_be(const byte* b) 
		{
			return (read16_be(b) << 16) + read16_be(b + 2);
		}

		uint32_t ReadDword( byte * & pData )
		{
			uint32_t value=read32_le(pData);
			pData+=4;
			return value;
		}

		/**
		* function to read in a header
		*/
		bool ReadHeader(FILE* pFile, DDS::DDSStruct& header)
		{
			const int headerSize=128;
			byte data[headerSize];
			byte * pData=data;
			size_t sizeRead = fread(pData, sizeof(char), headerSize, pFile);
			if( headerSize!=sizeRead )
			{
				// amount read different from amount required/expected
				return false;
			}

			// verify DDS files
			if (! (pData[0]=='D' && pData[1]=='D' && pData[2]=='S' && pData[3]==' ') )
			{
				return false;
			}
			pData+=4;

			header.size=ReadDword(pData);
			if (header.size!=124)
			{
				return false;
			}

			//convert the data
			header.flags=ReadDword(pData);
			header.height=ReadDword(pData);
			header.width=ReadDword(pData);
			header.sizeorpitch=ReadDword(pData);
			header.depth=ReadDword(pData);
			header.mipmapcount=ReadDword(pData);

			for (int i=0; i<11; ++i)
			{
				header.reserved[i]=ReadDword(pData);
			}
			
			//pixelfromat
			header.pixelformat.size=ReadDword(pData);
			header.pixelformat.flags=ReadDword(pData);
			header.pixelformat.fourCC=ReadDword(pData);
			header.pixelformat.RGBBitCount=ReadDword(pData);
			header.pixelformat.rBitMask=ReadDword(pData);
			header.pixelformat.gBitMask=ReadDword(pData);
			header.pixelformat.bBitMask=ReadDword(pData);
			header.pixelformat.alpahbitmask=ReadDword(pData);

			//caps
			header.ddscaps.caps1=ReadDword(pData);
			header.ddscaps.caps2=ReadDword(pData);
			header.ddscaps.reserved[0]=ReadDword(pData);
			header.ddscaps.reserved[1]=ReadDword(pData);
			header.reserved2=ReadDword(pData);

			return true;
		}

		public:
		LoaderImgData	imgdata_;
		bool			headerdone_;
		DDS::DDSStruct	surfacedata_;
	};

	// this is the function to call when we want to load
	// an image
	size_t ddsLoad(const char* filename, DDSImage& dds) 
	{
		// open the file for reading (binary mode)
		FILE* file = fopen(filename, "rb");
		if (file == nullptr) {
			return 0;
		}

		// read the dds file
		size_t sizeRead = dds.read(file);
		return sizeRead;
	}

#ifdef PICODDS_OPENGL
	GLuint createOglTexFromDDS(const char *pFilename)
	{
		GLuint texID = 0;
		PicoDDS::DDSImage img;
		size_t amtRead = PicoDDS::ddsLoad(pFilename, img);
		if(amtRead==0)
			return 0;

		const int oglInternalFormat = img.getOpenGLFormat();
		
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		// Generate The Texture
		if( 1 == img.imgdata_.numMipMaps ) {
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, oglInternalFormat, 
				img.imgdata_.width, img.imgdata_.height, 0, 
				img.imgdata_.size, img.imgdata_.imgData );
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, img.imgdata_.numMipMaps - 1);

			size_t Offset = 0;
			size_t Width = img.imgdata_.width;
			size_t Height = img.imgdata_.height;

			for( int32_t i=0; i<img.imgdata_.numMipMaps; ++i ) {
				size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * img.getMinDXTSize();

				glCompressedTexImage2D(GL_TEXTURE_2D, i, oglInternalFormat, 
					Width, Height, 0, bufSize, 
					&img.imgdata_.imgData[Offset] );

				Offset+=bufSize;
				if((Width/=2)==0) Width=1;
				if((Height/=2)==0) Height=1;
			}
		}
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		return texID;
	}
#endif // PICODDS_OPENGL
}
