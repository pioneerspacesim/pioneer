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

#include "PicoDDS.h"
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>

namespace PicoDDS
{
	
DDSImage::DDSImage() : headerdone_(false)
{
	memset(&surfacedata_,0,sizeof(surfacedata_));
}

DDSImage::DDSImage(DDSImage const &lhs) : imgdata_(lhs.imgdata_), headerdone_(lhs.headerdone_), surfacedata_(lhs.surfacedata_)
{
}

DDSImage::~DDSImage()
{
	if( imgdata_.imgData ) {
		delete [] imgdata_.imgData;
	}
}

size_t DDSImage::Read(const char* pData, const size_t dataSize)
{
	// Read in header and decode
	if (!ReadHeader(pData, surfacedata_))
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
	imgdata_.format = GetTextureFormat();
	imgdata_.numImages = GetNumImages();
	imgdata_.size = CalculateStoreageSize();
	if(0 >= imgdata_.size)
		return -1;
			
	if(-1 == imgdata_.format)
		return -1;

	const long headerSize=128;
	const size_t DDSStructSize = sizeof(DDS::DDSStruct)+4;
	// proceed with allocating memory and reading the file
	imgdata_.imgData = new byte[imgdata_.size];

	// Read in remaining data
	memcpy(imgdata_.imgData, pData + headerSize, dataSize-headerSize);

	return dataSize - headerSize;
}

int DDSImage::GetMinDXTSize() const
{
	return GetMinSize(GetDXTFormat());
}

int DDSImage::CalculateStoreageSize() const
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

ImgFormat DDSImage::GetTextureFormat() const
{
	ImgFormat format = FORMAT_NONE;

	if(surfacedata_.pixelformat.flags & DDS::DDPF_FOURCC)
	{
		format = GetDXTFormat();
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

int DDSImage::GetNumImages() const
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

ImgFormat DDSImage::GetDXTFormat() const
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
int DDSImage::GetOpenGLFormat()
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

// function to read in a header
bool DDSImage::ReadHeader(const char* pDataIn, DDS::DDSStruct& header)
{
	const int headerSize=128;
	byte data[headerSize];
	byte * pData=data;
	memcpy(pData, pDataIn, sizeof(byte) * headerSize);

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

	headerdone_ = true;
	return headerdone_;
}

// this is the function to call when we want to load an image
size_t DDSLoad(const char* filename, DDSImage& dds) 
{
	// open the file for reading (binary mode)
	FILE* file = fopen(filename, "rb");
	if (!file) {
		return 0;
	}

	// find the file size
	fseek(file, 0, SEEK_END); // seek to end of file
	const size_t size = ftell(file); // get current file pointer
	fseek(file, 0, SEEK_SET); // seek back to beginning of file

	// allocate space for the data and read the whole file in
	char* pData = (char*)malloc(sizeof(char)*size);
	if( pData ) {
		const size_t sizeRead = fread(pData, sizeof(char), size, file);
		assert(sizeRead == size);

		// read the dds file
		const size_t sizeDDSRead = dds.Read(pData, sizeRead);
		free(pData);
		return sizeDDSRead;
	}
	return 0;
}

}	// namespace PicoDDS
