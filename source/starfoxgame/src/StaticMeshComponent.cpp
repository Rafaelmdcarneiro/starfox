#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "DebugDraw.h"
#include "gs/Platform/GL/GLUtil.h"

extern bool g_drawNormals;
extern bool g_drawSockets;
extern float32 g_normalScale;

static void DrawStaticMesh(const gfx::StaticMesh& staticMesh, const Matrix43& mMeshToWorld)
{
	GLUtil::PushAndMultMatrix(mMeshToWorld);

	for (const auto& subMesh : staticMesh.m_subMeshes)
	{
		bool disabledTexturing = false;

		//glBindTexture(GL_TEXTURE_2D, subMesh.m_material.m_textureId);
		if (subMesh.m_materialIndex != gfx::StaticMesh::INVALID_MATERIAL_INDEX)
		{
			const gfx::Material& material = staticMesh.m_materials[subMesh.m_materialIndex];

			glMaterialfv(GL_FRONT, GL_AMBIENT, material.m_ambient.v);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, material.m_diffuse.v);
			glMaterialfv(GL_FRONT, GL_SPECULAR, material.m_specular.v);
			glMaterialfv(GL_FRONT, GL_EMISSION, material.m_emmissive.v);
			glMaterialf(GL_FRONT, GL_SHININESS, material.m_shininess);

			if (material.m_textureId == INVALID_TEXTURE_ID)
			{
				disabledTexturing = true;
				glDisable(GL_TEXTURE_2D);
			}
			else
			{			
				GLUtil::SelectTexture(material.m_textureId);
			}
		}

		glBegin(GL_TRIANGLES);
		for (const auto& vertex : subMesh.m_vertices)
		{
			glColor4fv(vertex.color.v);
			assert(Vector3(vertex.normal).IsUnit() && "Normal must be unit length for lighting to work!");
			glNormal3fv((vertex.normal /** mMeshToWorld*/).v);
			glTexCoord2fv(vertex.textureCoords.v);
			glVertex3fv((vertex.position /** mMeshToWorld*/).v);
		}
		glEnd();

		if (disabledTexturing)
			glEnable(GL_TEXTURE_2D);

		// Draw normals
		if (g_drawNormals)
		{
			glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
			glColor4f(1.f, 1.f, 1.f, 1.f);
			for (const auto& vertex : subMesh.m_vertices)
			{
				glVertex3fv(vertex.position.v);
				glVertex3f(vertex.position.x + vertex.normal.x * g_normalScale, vertex.position.y + vertex.normal.y * g_normalScale, vertex.position.z + vertex.normal.z * g_normalScale);
			}
			glEnd();
			glPopAttrib();
		}
	}

	if (g_drawSockets)
	{
		for (auto& socket : staticMesh.m_sockets)
		{
			static float32 scale = 10.f;
			DebugDrawAxes(socket.m_matrix, scale);
		}
	}

	glPopMatrix();
}

void StaticMeshComponent::Render()
{
	DrawStaticMesh(*m_pStaticMesh, GetSceneNode()->GetLocalToWorld());
}
