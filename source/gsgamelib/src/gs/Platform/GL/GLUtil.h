#ifndef __GL_UTIL__
#define __GL_UTIL__

// Useful OpenGL utilities and wrappers

#include "GLHeaders.h"
#include "gs/Rendering/Color4.h"
#include "gs/Math/Matrix43.h"
#include "gs/Image/ImageData.h"
#include <cassert>

#ifndef _DEBUG
	#define ASSERT_NO_GL_ERROR() ((void)0)
#else
	#define ASSERT_NO_GL_ERROR() GLHelperInternal::AssertNoGLError();

	namespace GLHelperInternal
	{
		static void AssertNoGLError()
		{
			GLenum glError = glGetError();
			switch (glError)
			{
				case GL_NO_ERROR: break;
				case GL_INVALID_ENUM		: assert(glError != GL_INVALID_ENUM && "GL Error: Invalid enum"); break;
				case GL_INVALID_VALUE		: assert(glError != GL_INVALID_VALUE && "GL Error: Invalid value"); break;
				case GL_INVALID_OPERATION	: assert(glError != GL_INVALID_OPERATION && "GL Error: Invalid operation"); break;
				case GL_STACK_OVERFLOW		: assert(glError != GL_STACK_OVERFLOW && "GL Error: Stack overflow"); break;
				case GL_STACK_UNDERFLOW		: assert(glError != GL_STACK_UNDERFLOW && "GL Error: Stack underflow"); break;
				case GL_OUT_OF_MEMORY		: assert(glError != GL_OUT_OF_MEMORY && "GL Error: Out of memory"); break;
				default						: assert(glError && "GL Error: unknown code");
			}
		}
	}
#endif

#define SET_GL_PARAM(flag, bEnable) (bEnable? glEnable(flag) : glDisable(flag)); ASSERT_NO_GL_ERROR()


namespace MatrixMode
{
	enum Type
	{
		ModelView,
		Projection
	};
}

namespace ShadeModel
{
	enum Type
	{
		Flat,
		Smooth
	};
}

namespace DepthFunc
{
	enum Type
	{
		LessOrEqual
	};
}


namespace Winding
{
	enum Type
	{
		Clockwise,
		CounterClockwise
	};
}

namespace CullBackFace
{
	enum Type
	{
		False,
		True
	};
}

namespace RenderShape
{
	enum Type
	{
		Points,
		Lines,
		Triangles,
		TriangleStrip,
		Quads,
		Polygon,

		NumTypes
	};
}

namespace TextureFilter
{
	enum Type
	{
		Nearest,
		Linear
	};
}

namespace TextureWrap
{
	enum Type
	{
		Repeat,
		Clamp		
	};
}

namespace BlendFunc
{
	enum Type
	{
		Blend,		// src * srcAlpha + dst * (1 - srcAlpha), typical blend
		Add,		// src + dst, saturates
		AddAlpha,	// src * srcAlpha + dst, "lightens" based on src alpha

		NumTypes
	};
}

// Contains view volume projection planes and whether it defines a frustum (perspective) or a cube (orthographic)
struct ProjectionInfo
{
	float32 left, right, top, bottom, near, far;
	bool isFrustum;

	void SetPerspective(float32 vertFovAngle, float32 aspectRatio, float32 near=0.1f, float32 far=1000.0f, bool swapTopBottom=false)
	{
		isFrustum = true;

		this->near = near;
		this->far = far;

		top = tan(MathEx::DegToRad(vertFovAngle) * 0.5f) * near;
		bottom = -top;

		left = aspectRatio * bottom;
		right = aspectRatio * top;

		if (swapTopBottom)
		{
			std::swap(top, bottom);
		}
	}

	void SetFrustum(float32 left, float32 right, float32 bottom, float32 top, float32 near=0.1f, float32 far=1000.0f)
	{
		isFrustum = true;
		this->left = left;
		this->right = right;
		this->top = top;
		this->bottom = bottom;
		this->near = near;
		this->far = far;
	}

	void SetOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near=-1.0f, float32 far=1.0f)
	{
		isFrustum = false;
		this->left = left;
		this->right = right;
		this->top = top;
		this->bottom = bottom;
		this->near = near;
		this->far = far;
	}
};


typedef int TextureId;
const TextureId INVALID_TEXTURE_ID = -1;

namespace GLUtil
{
	///////////////////////////////
	// General functions
	///////////////////////////////

	// Sets/gets current color, used by many other functions
	inline void SetColor(const Color4F& color)
	{
		glColor4f(color.r, color.g, color.b, color.a);
	}

	inline Color4F GetColor()
	{
		GLfloat colors[4];
		glGetFloatv(GL_CURRENT_COLOR, (GLfloat*)&colors);
		return Color4F(colors[0], colors[1], colors[2], colors[3]);
	}

	// Shading model
	inline void SetShadeModel(ShadeModel::Type model)
	{
		glShadeModel(model==ShadeModel::Smooth? GL_SMOOTH : GL_FLAT);
		ASSERT_NO_GL_ERROR();
	}

	inline ShadeModel::Type GetShadeModel()
	{
		GLint shadeModel;
		glGetIntegerv(GL_SHADE_MODEL, &shadeModel);
		ASSERT_NO_GL_ERROR();
		return shadeModel == GL_SMOOTH? ShadeModel::Smooth : ShadeModel::Flat;
	}

	// Sets depth (Z) testing, specifying the depth test function and the buffer clearing value (range [0,1])
	inline void SetDepthTesting(bool bEnable, DepthFunc::Type depthFunc = DepthFunc::LessOrEqual, float32 clearVal = 1.0f)
	{
		SET_GL_PARAM(GL_DEPTH_TEST, bEnable);
		if (bEnable)
		{			
			switch (depthFunc)
			{
				case DepthFunc::LessOrEqual: glDepthFunc(GL_LEQUAL); break;
				default: assert(false && "Unsupported depth function");
			}

			glClearDepth(clearVal);
			ASSERT_NO_GL_ERROR();
		}
	}

	// Clears color buffer
	inline void ClearColorBuffer(const Color4F& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Clears the depth (Z) buffer to clearVal set in SetDepthTesting()
	inline void ClearDepthBuffer()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	// Sets the drawing order (winding) of polygon front-faces and whether or not to cull
	// the backfaces.
	inline void SetFrontFace(Winding::Type winding, CullBackFace::Type cullBackFace)
	{
		glFrontFace(winding==Winding::Clockwise? GL_CW : GL_CCW);
		ASSERT_NO_GL_ERROR();
		
		// Enable/disable culling and make sure to set it to back-face if enabled
		SET_GL_PARAM(GL_CULL_FACE, cullBackFace==CullBackFace::True);

		if (cullBackFace == CullBackFace::True)
		{
			glCullFace(GL_BACK);
			ASSERT_NO_GL_ERROR();
		}
	}

	///////////////////////////////
	// General matrix functions
	///////////////////////////////

	// Call to change current matrix
	void MatrixMode(MatrixMode::Type mode, bool bLoadIdentity = true);	

	// Call to load identity matrix in current matrix
	inline void LoadIdentity()
	{
		glLoadIdentity();
	}

	// Pushes current matrix
	inline void PushMatrix()
	{
		glPushMatrix();
	}

	// Pops current matrix
	inline void PopMatrix()
	{
		glPopMatrix();
	}

	inline void Matrix43ToGLMatrix(const Matrix43& m43, GLfloat mGL[16])
	{
		auto& m = m43.m;
		mGL[0]  =  m[0][0];	mGL[1]  =  m[0][1];	mGL[2]  =  m[0][2];	mGL[3]  = 0.f;
		mGL[4]  =  m[1][0];	mGL[5]  =  m[1][1];	mGL[6]  =  m[1][2];	mGL[7]  = 0.f;
		mGL[8]  =  m[2][0];	mGL[9]  =  m[2][1];	mGL[10] =  m[2][2];	mGL[11] = 0.f;
		mGL[12] =  m[3][0];	mGL[13] =  m[3][1];	mGL[14] =  m[3][2];	mGL[15] = 1.f;
	}

	// Replaces current matrix at top of stack with input one
	inline void LoadMatrix(const Matrix43& m)
	{
		GLfloat mNodeGL[16];
		Matrix43ToGLMatrix(m, mNodeGL);
		glLoadMatrixf( mNodeGL );
	}

	// Pre-multiplies input matrix by matrix at top of stack, replacing
	// the top matrix with the result.
	inline void PreMultiplyMatrix(const Matrix43& m)
	{
		GLfloat mNodeGL[16];
		Matrix43ToGLMatrix(m, mNodeGL);
		glMultMatrixf( mNodeGL );
	}

	inline void PushAndMultMatrix(const Matrix43& mMeshToWorld)
	{
		glPushMatrix();
		GLfloat mNodeGL[16];
		GLUtil::Matrix43ToGLMatrix(mMeshToWorld, mNodeGL);
		glMultMatrixf(mNodeGL);
	}

	///////////////////////////////
	// Modelview matrix functions
	///////////////////////////////

	// Translates to specified (x,y,z) offset from current position
	inline void Translate(float32 x, float32 y, float32 z)
	{
		glTranslatef(x, y, z);
	}

	// Translates to specified (x,y) offset from current position
	inline void Translate(float32 x, float32 y)
	{
		glTranslatef(x, y, 0);
	}

	// Rotates angle amount in radians around specified axis
	inline void Rotate(const float32 rads, float32 x, float32 y, float32 z)
	{
		glRotatef(MathEx::RadToDeg(rads), x, y, z);
	}

	// Rotates angle amount in radians around z-axis at current position
	inline void Rotate(const float32 rads)
	{
		glRotatef(MathEx::RadToDeg(rads), 0, 0, 1);
	}

	// Scales by input factors, which affects subsequent rendering
	inline void Scale(float32 x, float32 y)
	{
		glScalef(x, y, 0);
	}

	///////////////////////////////
	// Projection matrix functions
	///////////////////////////////

	// Sets up orthographic projection
	inline void Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
	{
		glOrtho(left, right, bottom, top, near, far);
	}

	// Sets up perspective projection where you define a frustrum
	inline void Perspective(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
	{
		glFrustum(left, right, bottom, top, near, far);
	}

	// Easier method to setup a perspective projection using field of view angle and aspect ratio
	inline void Perspective(float32 fieldOfViewAngle, float32 aspectRatio, float32 near, float32 far)
	{
		gluPerspective(fieldOfViewAngle, aspectRatio, near, far);
	}

	// Helper for 2D
	inline void Orthographic2D(float32 left, float32 right, float32 bottom, float32 top)
	{
		Orthographic(left, right, bottom, top, -1, 1);
	}

	inline void SetProjection(const ProjectionInfo& pi)
	{
		MatrixMode(MatrixMode::Projection);
		if (pi.isFrustum)
			Perspective(pi.left, pi.right, pi.bottom, pi.top, pi.near, pi.far);
		else
			Orthographic(pi.left, pi.right, pi.bottom, pi.top, pi.near, pi.far);
	}

	///////////////////////////////
	// Texturing functions
	///////////////////////////////

	// Enables/disables texturing
	inline void SetTexturing(bool bEnable)
	{
		SET_GL_PARAM(GL_TEXTURE_2D, bEnable);
	}

	inline bool GetTexturing()
	{
		return glIsEnabled(GL_TEXTURE_2D) == GL_TRUE;
	}

	// Creates a 2d texture, returning unique texture id and the texture's size in
	// the output parameter. Consecutive calls guarantee returning consecutive ids
	// (i.e. 3,4,5,etc.). The texture size might be larger if the image size is not
	// a power of two.
	TextureId LoadTexture(const ImageData& imgData, Size2d<int>& texSize);
	TextureId LoadTexture(const ImageData& imgData);

	// Unloads the texture data for the input texture id
	inline void FreeTexture(TextureId& rTexId)
	{
		unsigned int tex = rTexId;
		glDeleteTextures(1, &tex);
	}

	// Select/unselects the input texture
	inline void SelectTexture(const TextureId& rTexId)
	{
		glBindTexture(GL_TEXTURE_2D, rTexId);
	}

	// These functions are only valid for currently selected texture...

	// Sets filtering method
	inline void SetTextureFilter(TextureFilter::Type minFilter, TextureFilter::Type magFilter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter == TextureFilter::Nearest? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == TextureFilter::Nearest? GL_NEAREST : GL_LINEAR);
	}

	// Sets wrapping method
	inline void SetTextureWrap(TextureWrap::Type sWrapType, TextureWrap::Type tWrapType)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrapType == TextureWrap::Repeat? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrapType == TextureWrap::Repeat? GL_REPEAT : GL_CLAMP);
	}

	///////////////////////////////
	// Rendering functions
	///////////////////////////////

	inline void BeginRender(RenderShape::Type shape)
	{
		static GLenum renderShapeMapping[] =
		{
			GL_POINTS,
			GL_LINES,
			GL_TRIANGLES,
			GL_TRIANGLE_STRIP,
			GL_QUADS,
			GL_POLYGON
		};
		// If this assert is triggered, it likely means we need to update the mapping array just above
		static_assert(ARRAY_SIZE(renderShapeMapping) == RenderShape::NumTypes, "Mismatched array size");
	
		ASSERT_NO_GL_ERROR(); // Can't check between calls to glBegin() and glEnd()
		glBegin(renderShapeMapping[shape]);
	}

	inline void EndRender()
	{
		glEnd();
		ASSERT_NO_GL_ERROR(); // Can't check between calls to glBegin() and glEnd()
	}

	inline void Vertex(float32 x, float32 y)
	{
		glVertex2f(x, y);
	}

	inline void Vertex(float32 x, float32 y, float32 z)
	{
		glVertex3f(x, y, z);
	}

	inline void TexCoord(float32 s, float32 t)
	{
		glTexCoord2f(s, t);
	}

	// Useful for debugging, renders the x-y-z axes
	// (x=red, y=blue, z=green)
	// NOTE: Make sure texturing is disabled before calling this function.
	inline void RenderXYZAxes(float32 fAxisSize=1.0f, float32 fAxisWidth=1.0f)
	{
		float32 currColor[4];
		glGetFloatv(GL_CURRENT_COLOR, currColor); // Save current color

		float32 fCurrLineWidth;
		glGetFloatv(GL_LINE_WIDTH, &fCurrLineWidth); // Save current line width

		glLineWidth(fAxisWidth); // Set line width

		glBegin(GL_LINES);		
		// x-axis
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(fAxisSize, 0.0f, 0.0f);

		// y-axis
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, fAxisSize, 0.0f);

		// z-axis
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, fAxisSize);
		glEnd();

		glColor4fv(currColor); // Restore color
		glLineWidth(fCurrLineWidth); // Restore line width
	}

	// Helper that renders four vertices of a quad
	inline void RenderQuad(float32 x, float32 y, float32 w, float32 h)
	{
		float32 x2 = x+w;
		float32 y2 = y+h;

		Vertex(x, y);
		Vertex(x2, y);
		Vertex(x2, y2);
		Vertex(x, y2);
	}

	// Sets whether to render all in wireframe
	inline void SetWireFrame(bool bEnable)
	{
		glPolygonMode(GL_FRONT_AND_BACK, bEnable? GL_LINE : GL_FILL);
	}

	//////////////////////////////////////
	// Blending functions
	//////////////////////////////////////

	// Enable/disable blending
	inline void SetBlending(bool bEnable)
	{
		SET_GL_PARAM(GL_BLEND, bEnable);
	}

	// Sets blending function
	inline void SetBlendFunc(BlendFunc::Type func)
	{
		static struct { GLenum srcFunc; GLenum dstFunc; } blendFuncMapping[] =
		{
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }, // Blend
			{ GL_ONE, GL_ONE }, // Add
			{ GL_SRC_ALPHA, GL_ONE }, // AddAlpha
		};
		// If this assert is triggered, it likely means we need to update the mapping array just above		
		static_assert(ARRAY_SIZE(blendFuncMapping) == BlendFunc::NumTypes, "Mismatched array size");

		glBlendFunc(blendFuncMapping[func].srcFunc, blendFuncMapping[func].dstFunc);
		ASSERT_NO_GL_ERROR();
	}

	//////////////////////////////////////
	// Lighting functions
	//////////////////////////////////////

	inline void SetLighting(bool bEnable)
	{
		SET_GL_PARAM(GL_LIGHTING, bEnable);
	}

} // namespace GLUtil

#undef SET_GL_PARAM

#endif // __GL_UTIL__