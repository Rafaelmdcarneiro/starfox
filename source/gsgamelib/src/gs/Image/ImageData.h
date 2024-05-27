#ifndef _IMAGE_DATA_H_
#define _IMAGE_DATA_H_

#include "gs/Rendering/Color4.h"
#include <string>
#include <memory>

typedef unsigned char UBYTE;

template <typename T>
class Size2d
{
public:
	T w, h;

	Size2d() : w(T()), h(T())			{}
	Size2d(T W, T H) : w(W), h(H)		{}

	// Explicit conversion constructor - must cast to T2:
	// Size2d<float32> f;
	// Size2d<int> i = static_cast<Size2d<int> >(f); // cast is necessary
	template <typename T2>
	explicit Size2d(Size2d<T2> const& rhs)
		: w( static_cast<T>(rhs.w) ), h( static_cast<T>(rhs.h) )
	{
	}

	void Set(T W, T H)					{ w = W; h = H; }
	void Get(T& W, T& H)				{ H = h; W = w; }
};

// Basic image information struct
struct ImageInfo
{
	ImageInfo() : iChannels(0), imageSize(0,0) {}

	// Helpers
	int GetChannels() const		{ return iChannels; }
	int GetWidth() const		{ return imageSize.w; }
	int GetHeight() const		{ return imageSize.h; }
	int GetPitch() const		{ return iChannels * imageSize.w; }
	int GetDataSize() const		{ return GetPitch() * imageSize.h; }

	int iChannels;			// Channels in the image
	Size2d<int> imageSize;	// Image size (width and height)
};


// Encapsulates a single image including its data and format information.
class ImageData
{
	public: // Static functions that return ImageData objects

		// Generic load function - determines function to call based on file extension
		static ImageData Load(const std::string& strFileName);

		// Specific file-format load functions	
		static ImageData LoadTGA(const std::string& strFileName);
		static ImageData LoadRAW(const std::string& strFileName, int iWidth, int iHeight, int iChannels=3);

	public:		
		~ImageData();

		ImageData(const ImageData& rhs);			// Copy constructor
		ImageData& operator=(const ImageData& rhs);	// Assignment operator

		// Adds an alpha channel, setting it all to white (1) except for the input
		// RGB color key value, which is set to black (0).
		void AddAlphaChannel(const Color4UB& color = Color4UB::Black());

		// Typical accessors
		const std::string& GetFileName() const { return m_strFileName; }
		int GetChannels() const			{ return m_imageInfo.iChannels; }		
		int GetWidth() const			{ return m_imageInfo.imageSize.w; }
		int GetHeight() const			{ return m_imageInfo.imageSize.h; }
		Size2d<int> GetSize() const		{ return m_imageInfo.imageSize; }
		ImageInfo GetImageInfo() const	{ return m_imageInfo; }

		// Returns pointer to data buffer
		UBYTE* GetData() const			{ return m_pData.get(); }

		// Returns size in bytes of data buffer
		int GetDataSize() const			{ return GetChannels() * GetHeight() * GetWidth(); }

		// Returns the pitch - the number of bytes between first 
		// pixel of one row to first pixel of next row (byte size of one row)
		int GetPitch() const			{ return GetChannels() * GetWidth(); }

		// Copies data from source buffer to destination buffer
		void CopyDataTo(UBYTE* const pDstData) const;
		void CopyDataTo(UBYTE* const pDstData, int srcX, int srcY, int srcWidth, int srcHeight) const;

		// Returns a ImageData that is a sub-rect of this one.
		// If bCopy is true, the data in the returned ImageData is a copy of this one's.
		// If bCopy is false, the returned ImageData contains a reference to this one's data.
		ImageData GetSubImageData(int srcX, int srcY, int srcWidth, int srcHeight, bool bCopy=true) const;

		// Reverses the rows of data, essentially flipping it upside down
		ImageData& ReverseRows();

		// Scales the data by the positive integer amounts in either direction
		ImageData& Scale(unsigned int iWidthScaleFactor, unsigned int iHeightScaleFactor);

	private:
		ImageData();
		ImageData(const std::string& strFileName, const ImageInfo& rImageInfo, std::shared_ptr<UBYTE> pData);

		/////////////////////////////////////////////////////////////////////////////
		// NOTE: This class has a copy constructor/assignment operator function.
		// If new data members are added, update these functions.
		/////////////////////////////////////////////////////////////////////////////

		std::string m_strFileName;			// The image file name
		ImageInfo m_imageInfo;				// Image information
		std::shared_ptr<UBYTE> m_pData;		// Pointer to image data buffer
};

#endif // _IMAGE_DATA_H_
