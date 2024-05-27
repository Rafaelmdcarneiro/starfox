#include "GroundComponent.h"
#include "gs/Platform/GL/GLUtil.h"

void GroundComponent::Render()
{
	const Matrix43& mWorld = GetSceneNode()->GetLocalToWorld();

	// Draw the ground plane
	TWEAKABLE float32 halfPlaneSizeX = 10000.f;
	TWEAKABLE float32 planeStartZ = 1000.f;
	TWEAKABLE float32 planeEndZ = 9000.f;

	const Vector3& v1 = PositionVector(Vector3(-halfPlaneSizeX, 0.f, -planeStartZ)) * mWorld;
	const Vector3& v2 = PositionVector(Vector3(halfPlaneSizeX, 0.f, -planeStartZ)) * mWorld;
	const Vector3& v3 = PositionVector(Vector3(halfPlaneSizeX, 0.f, planeEndZ)) * mWorld;
	const Vector3& v4 = PositionVector(Vector3(-halfPlaneSizeX, 0.f, planeEndZ)) * mWorld;

	glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
		
	glBegin(GL_QUADS);
	{
		glColor3ub(25, 89, 58);
		glVertex3fv(v1.v);
		glVertex3fv(v2.v);

		glColor3ub(132, 178, 181);
		glVertex3fv(v3.v);
		glVertex3fv(v4.v);
	}
	glEnd();

	// Draw visible white dots
	TWEAKABLE float32 halfDistX = 80.f;
	TWEAKABLE float32 distZ = 200.f;
	TWEAKABLE float32 numDots = 20.f;

	const Vector3 vOffsetX(halfDistX, 0.f, 0.f);

	Vector3 vDotCenter = mWorld.trans;
	vDotCenter.x = 0.f;
	vDotCenter.y += 0.1f;
	vDotCenter.z = MathEx::Floor(vDotCenter.z / distZ) * distZ;
		
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	for (uint32 i = 0; i < numDots; ++i)
	{
		glVertex3fv((vDotCenter - vOffsetX).v);
		glVertex3fv((vDotCenter - vOffsetX * 3.f).v);
		glVertex3fv((vDotCenter - vOffsetX * 5.f).v);
		glVertex3fv((vDotCenter - vOffsetX * 7.f).v);

		glVertex3fv((vDotCenter + vOffsetX).v);
		glVertex3fv((vDotCenter + vOffsetX * 3.f).v);
		glVertex3fv((vDotCenter + vOffsetX * 5.f).v);
		glVertex3fv((vDotCenter + vOffsetX * 7.f).v);

		vDotCenter.z += distZ;
	}
	glEnd();

	glPopAttrib();
	//glPopMatrix();
}
