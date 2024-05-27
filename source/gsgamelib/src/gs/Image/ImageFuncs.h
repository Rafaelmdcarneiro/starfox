#ifndef _IMAGE_FUNCS_H_
#define _IMAGE_FUNCS_H_

#include "gs/Rendering/Color4.h"
#include "ImageData.h"

#include <string>
#include <memory>

struct ImageInfo;
typedef unsigned char UBYTE;

namespace ImageFuncs
{
	// Loads TGA file, returning allocated data and filling up rImageInfo
	std::shared_ptr<UBYTE> LoadTGA(const std::string& strFileName, ImageInfo& rImageInfo);

	// Loads up RAW file, returning allocate data and filling up rImageInfo
	std::shared_ptr<UBYTE> LoadRAW(
		const std::string& strFileName,
		int iWidth, int iHeight, int iChannels,
		ImageInfo& rImageInfo);

	// Modifies rpData by adding an alpha channel, setting it all to white (1) except
	// for the input RGB color key value, which is set to black (0). rImageInfo is
	// also updated (iChannels is set to 4).
	// NOTE: both rpData and rImageInfo are in/out parameters.
	void AddAlphaChannel(
		std::shared_ptr<UBYTE>& rpData, ImageInfo& rImageInfo,
		const Color4UB& color);

	// Modifies rpData by adding an all white alpha channel. This is useful when you
	// need an alpha channel, but no transparency (used by GrowToPowerOf2)
	UBYTE* AddWhiteAlphaChannel(const UBYTE* pData, ImageInfo& rImageInfo);
	
	// Modifies pData by reversing all the rows
	void ReverseRows(UBYTE* pData, const ImageInfo& rImageInfo);

	// If input image must be "grown", this function returns a newly allocated
	// byte array (which you must delete) containing the grown data. An alpha
	// channel is created if it does not already exist, and is cleared to black (0)
	// for the "grown" portion. If input image already has power of 2 dimensions,
	// the function returns nullptr
	UBYTE* GrowToPowerOf2(const UBYTE* pData, ImageInfo& rImageInfo);

	bool IsPowerOf2(const Size2d<int>& rSize);

} // namespace ImageFuncs

#endif // _IMAGE_FUNCS_H_
