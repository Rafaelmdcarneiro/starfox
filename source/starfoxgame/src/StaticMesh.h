#ifndef __STATIC_MESH_H__
#define __STATIC_MESH_H__

#include "gs/Base/Base.h"
#include "gs/Math/Matrix43.h"
#include "gs/Platform/GL/GLUtil.h"
#include <vector>

namespace gfx
{

struct Color4 : Vector4
{
	Color4(float32 x = 1.f, float32 y = 1.f, float32 z = 1.f, float32 w = 1.f)
		: Vector4(x, y, z, w) {}
};

struct Vector2
{
	Vector2() {}

	Vector2(float32 x, float32 y)
		: x(x), y(y) {}

	union
	{
		struct { float32 x, y; };
		float32 v[2];
	};
};

struct Material
{
	Material() 
		: m_uniqueId(~0ul)
		, m_shininess(0.f)
		, m_textureId(INVALID_TEXTURE_ID)
	{}

	uint64 m_uniqueId;
	std::string m_name;
	std::string m_filename;

	Color4 m_ambient;
	Color4 m_diffuse;
	Color4 m_specular;
	Color4 m_emmissive;
	float32 m_shininess;

	TextureId m_textureId;
};

struct StaticMesh
{
	struct Vertex
	{
		Vertex()
			: position(0,0,0,1)
			, normal(0,0,0,0)
			, color(1,1,1,1)
			, textureCoords(0,0)
		{
		}

		Vector4 position;
		Vector4 normal;
		Color4 color;
		Vector2 textureCoords;
	};

	static const uint32 INVALID_MATERIAL_INDEX = ~0u;

	struct SubMesh
	{
		SubMesh() : m_materialIndex(INVALID_MATERIAL_INDEX) {}
		std::vector<Vertex> m_vertices;
		uint32 m_materialIndex;
	};

	struct Socket
	{
		std::string m_name;
		Matrix43 m_matrix;
	};

	std::vector<SubMesh> m_subMeshes;
	std::vector<Material> m_materials;
	std::vector<Socket> m_sockets;

	struct BoundingBox
	{
		BoundingBox() : m_min(Vector3::Zero()), m_max(Vector3::Zero()) {}

		void Include(const Vector3& v)
		{
			m_min.x = MathEx::Min(m_min.x, v.x);
			m_min.y = MathEx::Min(m_min.y, v.y);
			m_min.z = MathEx::Min(m_min.z, v.z);
			m_max.x = MathEx::Max(m_max.x, v.x);
			m_max.y = MathEx::Max(m_max.y, v.y);
			m_max.z = MathEx::Max(m_max.z, v.z);
		}

		Vector3 GetExtents()
		{
			return m_max - m_min;
		}

		Vector3 m_min, m_max;
	};
	BoundingBox m_boundingBox;
};

} // namespace gfx

#endif // __STATIC_MESH_H__
