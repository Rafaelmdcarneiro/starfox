#include "ImageData.h"

#include "gs/Base/string_helpers.h"
#include "ImageFuncs.h" // Internal
#include <stdio.h>
#include <stdexcept>
#include <cassert>

using namespace std;

ImageData ImageData::Load(const std::string& strFileName)
{
	assert(strFileName.size()>0 && "File name string is empty");

	size_t pos = strFileName.rfind('.');
	if ( pos != string::npos )
	{
		string strExt = strFileName.substr(pos+1);
	
		if ( str_compare_no_case(strExt, "tga")==0 )
			return LoadTGA(strFileName);
	}

	throw std::invalid_argument("Unsupported graphics file format:" + strFileName);
}

ImageData ImageData::LoadTGA(const std::string& strFileName)
{
	ImageInfo imageInfo;
	std::shared_ptr<UBYTE> pData = ImageFuncs::LoadTGA(strFileName, imageInfo);
	return ImageData(strFileName, imageInfo, pData);
}

ImageData ImageData::LoadRAW(const std::string& strFileName, int iWidth, int iHeight, int iChannels/*=3*/)
{
	ImageInfo imageInfo;
	std::shared_ptr<UBYTE> pData = ImageFuncs::LoadRAW(strFileName, iWidth, iHeight, iChannels, imageInfo);
	return ImageData(strFileName, imageInfo, pData);
}


ImageData::ImageData()
{
}

// Protected constructor called by static functions
ImageData::ImageData(const std::string& strFileName, const ImageInfo& rImageInfo, std::shared_ptr<UBYTE> pData)
: m_strFileName(strFileName), m_imageInfo(rImageInfo), m_pData(pData)
{
}

ImageData::~ImageData()
{
}

ImageData::ImageData(const ImageData& rhs)
{
	*this = rhs;
}

ImageData& ImageData::operator=(const ImageData& rhs)
{
	if (&rhs != this)
	{
		m_strFileName = rhs.m_strFileName;
		m_imageInfo = rhs.m_imageInfo;

		// Allocate our own memory for this image and copy contents
		// of rhs image into it
		m_pData.reset( new UBYTE[rhs.GetDataSize()], std::default_delete<UBYTE[]>() );
		rhs.CopyDataTo(m_pData.get());
	}
	return *this;
}

void ImageData::AddAlphaChannel(const Color4UB& color)
{
	ImageFuncs::AddAlphaChannel(m_pData, m_imageInfo, color);
}

void ImageData::CopyDataTo(UBYTE* const pDstData) const
{
	memcpy(pDstData, GetData(), GetDataSize());
}

void ImageData::CopyDataTo(UBYTE* const pDstData, int srcX, int srcY, int srcWidth, int srcHeight) const
{
	// Set src pointer to first pixel of source buffer
	UBYTE* pSrcDataPtr = GetData();

	// Set dest pointer to first pixel of destination buffer
	UBYTE* pDstDataPtr = pDstData;

	// Calculate destination buffer pitch (num of bytes in one row of dst buffer)
	unsigned int dstPitch = srcWidth * GetChannels();

	// Move source pointer to first pixel of data we need to copy
	pSrcDataPtr += (srcY * GetPitch()) + (srcX * GetChannels());

	// Now loop and copy into destination buffer, updating pointers
	for (int i=0; i<srcHeight; ++i)
	{
		memcpy(pDstDataPtr, pSrcDataPtr, dstPitch); // Copy one row
		pSrcDataPtr += GetPitch();	// Move src pointer to next row
		pDstDataPtr += dstPitch;	// Move dst pointer to next row
	}
}

ImageData ImageData::GetSubImageData(int srcX, int srcY, int srcWidth, int srcHeight, bool bCopy/*=true*/) const
{
	assert(bCopy == true && "Refs not implemented yet");

	// Allocate memory for the data
	std::shared_ptr<UBYTE> pDstData( new UBYTE[srcHeight * srcWidth * GetChannels()], std::default_delete<UBYTE[]>() );

	// Now copy the data
	CopyDataTo(pDstData.get(), srcX, srcY, srcWidth, srcHeight);

	// Return brand new ImageData
	ImageInfo imageInfo;
	imageInfo.iChannels = GetChannels();
	imageInfo.imageSize = Size2d<int>(srcWidth, srcHeight);

	return ImageData(m_strFileName, imageInfo, pDstData);
}

ImageData& ImageData::ReverseRows()
{
	ImageFuncs::ReverseRows(m_pData.get(), m_imageInfo);
	return *this;
}

ImageData& ImageData::Scale(unsigned int iWidthScaleFactor, unsigned int iHeightScaleFactor)
{
	assert(iWidthScaleFactor>0 && iHeightScaleFactor>0 && "Scale factors must be positive");
	
	if ( iWidthScaleFactor == 1 && iHeightScaleFactor == 1 )
		return *this; // Nothing to do

	// Allocate larger data buffer
	int iScaledBuffSizeBytes = m_imageInfo.GetDataSize() * iWidthScaleFactor * iHeightScaleFactor;
	std::shared_ptr<UBYTE> pScaledData( new UBYTE[iScaledBuffSizeBytes], std::default_delete<UBYTE[]>() );

	UBYTE* pSrc = m_pData.get();
	UBYTE* pDst = pScaledData.get();	

	int iChunkSize = m_imageInfo.GetChannels();	// In bytes
	int iSrcRowSize   = m_imageInfo.GetPitch();	// In bytes
	int iSrcRowChunks = iSrcRowSize / iChunkSize;

	int iDstRowSize = iSrcRowSize * iWidthScaleFactor;

	// For each row in source buffer...
	for (int iRow=0; iRow<m_imageInfo.GetHeight(); ++iRow)
	{
		// First scale one row into destination
		for (int iChunk=0; iChunk<iSrcRowChunks; ++iChunk)
		{
			for (unsigned int i=0; i<iWidthScaleFactor; ++i)
			{
				memcpy(pDst, pSrc, iChunkSize); // Copy iChunkSize bytes
				pDst += iChunkSize;
			}
			pSrc += iChunkSize;
		}

		// When we reach here, pSrc and pDst should point to start of next row in
		// their respective buffers. Now we just blast the row we just created in
		// the destination buffer height scale factor - 1 times into destination.
		for (unsigned int i=1; i<iHeightScaleFactor; ++i)
		{
			memcpy(pDst, pDst-iDstRowSize, iDstRowSize); // Copy one row
			pDst += iDstRowSize;
		}
	}
	pSrc = pDst = nullptr; // No longer valid, don't use these anymore

	// Replace data pointer (old m_pData automatically deleted)
	m_pData = pScaledData;

	// Update size member
	m_imageInfo.imageSize.w *= iWidthScaleFactor;
	m_imageInfo.imageSize.h *= iHeightScaleFactor;

	return *this;
}
