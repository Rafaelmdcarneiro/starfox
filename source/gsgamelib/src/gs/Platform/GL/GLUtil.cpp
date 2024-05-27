#include "GLUtil.h"
#include "gs/Image/ImageFuncs.h"

namespace GLUtil {

void MatrixMode(MatrixMode::Type mode, bool bLoadIdentity)
{
	switch (mode)
	{
		case MatrixMode::ModelView:		glMatrixMode(GL_MODELVIEW); break;
		case MatrixMode::Projection:	glMatrixMode(GL_PROJECTION); break;
		default: assert(false && "Unexpected matrix mode");
	}

	if (bLoadIdentity)
		LoadIdentity();
}

TextureId LoadTexture(const ImageData& imgData, Size2d<int>& texSize)
{
	// Make our own copy of the data because we need to manipulate it
	std::unique_ptr<uint8[]> pDataCopy( new uint8[imgData.GetDataSize()] );
	imgData.CopyDataTo(pDataCopy.get());	

	// And a copy of the image info
	ImageInfo imageInfoCopy = imgData.GetImageInfo();	

#ifdef SHOW_TEX_MEM_USAGE
	static int iTotalBytes = 0;
	static int iUsedBytes = 0;
	static int iWastedBytes = 0;
	iUsedBytes += imageInfoCopy.GetDataSize();
#endif

	// NOTE: Not sure if this is necessary anymore. We can always just map textures upside
	// down ourselves.
#if 0
	// OpenGL stores textures upside-down, so we must invert the data...
	ImageFuncs::ReverseRows(pDataCopy.get(), imageInfoCopy); // Modifies pDataCopy
#endif

	// In OpenGL, textures must be a power of 2, so we must check the size of
	// the image and create a texture large enough to fit the image onto it
	uint8* pTemp = ImageFuncs::GrowToPowerOf2(pDataCopy.get(), imageInfoCopy); // May modify imageInfoCopy
	if ( pTemp != nullptr )
		pDataCopy.reset(pTemp);
/*
	assert(imageInfoCopy.GetWidth()<=256 && imageInfoCopy.GetHeight()<=256 && "Texture width or height > 256, may not be compatible with some video cards");
*/
#ifdef SHOW_TEX_MEM_USAGE
	iWastedBytes += imageInfoCopy.GetDataSize() - imgData.GetImageInfo().GetDataSize();
	iTotalBytes = iUsedBytes + iWastedBytes;

	using namespace std;
	cout << "Total Bytes: " << iTotalBytes
	<< "\tUsed Bytes: " << iUsedBytes << "(" << ((float32)iUsedBytes*100.0f/(float32)iTotalBytes) << "%)"
	<< "\tWasted Bytes: " << iWastedBytes << "(" << ((float32)iWastedBytes*100.0f/(float32)iTotalBytes) << "%)" << endl;
#endif

	// Finally, generate texture id and load data into it	
	GLuint uiTexId;
	glGenTextures(1, &uiTexId);
	TextureId texId = uiTexId;

	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D, 0, imageInfoCopy.iChannels, imageInfoCopy.imageSize.w,
		imageInfoCopy.imageSize.h, 0, (imageInfoCopy.iChannels==3? GL_RGB:GL_RGBA), 
		GL_UNSIGNED_BYTE, pDataCopy.get());

	// If we assert here, then something went wrong while loading
	// the texture into texture memory
	ASSERT_NO_GL_ERROR();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set output texture image size and return texture id
	texSize = imageInfoCopy.imageSize;
	return texId;
}

TextureId LoadTexture(const ImageData& imgData)
{
	Size2d<int> dummy;
	return LoadTexture(imgData, dummy);
}

} // namespace GLUtil {