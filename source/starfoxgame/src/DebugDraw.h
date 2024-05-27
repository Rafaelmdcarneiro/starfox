#ifndef _DEBUG_DRAW_H_
#define _DEBUG_DRAW_H_

#include "gs/Math/Vector3.h"
#include "gs/Math/Matrix43.h"
#include "gs/Rendering/Color4.h"
#include "gs/Platform/GL/GLHeaders.h"
#include <vector>

class DebugDrawManager
{
public:
	struct Line { Vector3 v1, v2; Color4F color; };

	void AddLine(const Line& line) { m_lines.push_back(line); }

	void Clear()
	{
		m_lines.clear();
	}

	void Render()
	{
		glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		
		
		glBegin(GL_LINES);
		for (const Line& line : m_lines)
		{
			glColor4fv(line.color.v);
			glVertex3fv(line.v1.v);
			glVertex3fv(line.v2.v);
		}
		glEnd();
		
		
		glPopAttrib();

		Clear();
	}

private:
	std::vector<Line> m_lines;
};

extern DebugDrawManager g_debugDrawManager;

inline void DebugDrawLine(const Vector3& v1, const Vector3& v2, const Color4F& color = Color4F::White())
{
	DebugDrawManager::Line line = {v1, v2, color};
	g_debugDrawManager.AddLine( line );
}

inline void DebugDrawAxes(const Matrix43& m, float32 scale = 1.f)
{
	DebugDrawLine(m.trans, m.trans + m.axisX * scale, Color4F::Red());
	DebugDrawLine(m.trans, m.trans + m.axisY * scale, Color4F::Green());
	DebugDrawLine(m.trans, m.trans + m.axisZ * scale, Color4F::Blue());
}

#endif // _DEBUG_DRAW_H_
