#include "AnchorComponent.h"

AnchorComponent::AnchorComponent()
	: m_totalDistance(0.f)
{
}

void AnchorComponent::Advance(float32 distance)
{
	m_totalDistance += distance;

	// For now, just move along Z axis
	auto& mLocal = GetSceneNode()->ModifyLocalToParent();
	mLocal.trans += mLocal.axisZ * distance;

	//TWEAKABLE float32 speed = 0.005f;
	//TWEAKABLE float32 amplitude = 100.f;
	//const float32 lastS = MathEx::Sin(speed * (m_totalDistance - distance));
	//const float32 currS = MathEx::Sin(speed * m_totalDistance);
	//const float32 delta = (lastS - currS) * amplitude;
	//mLocal.trans += mLocal.axisX * delta;
}
