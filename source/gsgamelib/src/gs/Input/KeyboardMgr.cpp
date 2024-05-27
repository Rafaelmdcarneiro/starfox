#include "KeyboardMgr.h"

///////////////////////////////////////////////////////////////////////////////
// KeyState implementation
///////////////////////////////////////////////////////////////////////////////
KeyState::Type KeyState::GetState() const
{
	if (JustPressed())
	{
		return KeyState::Pressed;
	}
	if (JustReleased())
	{
		return KeyState::Released;
	}
	if (IsDown())
	{
		return KeyState::Down;
	}

	assert(IsUp());
	return KeyState::Up;
}


///////////////////////////////////////////////////////////////////////////////
// KeyboardMgr implementation
///////////////////////////////////////////////////////////////////////////////

void KeyboardMgr::Update(float32 frameTimeMS)
{
	for (VKEY vkey = 0; vkey < m_keyStateMap.size(); ++vkey)
	{
		UpdateKeyState(vkey, frameTimeMS);
	}
}

void KeyboardMgr::SetAllKeysUp()
{
	for (auto& keyState : m_keyStateMap)
	{
		keyState.SetUp();
		keyState.SetUp();
	}
}

void KeyboardMgr::UpdateKeyState(VKEY vkey, float32 /*frameTimeMS*/)
{
	//assert(m_keyStateMap.find(vkey) != m_keyStateMap.end());

	if ( System::IsVKeyDown(vkey) )
	{
		m_keyStateMap[vkey].SetDown();
	}
	else
	{
		m_keyStateMap[vkey].SetUp();
	}
}

std::string KeyboardMgr::GetTypedInput()
{
	// This is some hacky attempt to get typed input.

	KeyboardMgr& kbMgr = *this;
	std::string output;

	// Check for alphabetic values with the Shift key
	for (char c = 'A'; c < 'Z'; ++c)
	{
		if (kbMgr[c].JustPressed())
		{
			if (!kbMgr[VK_SHIFT].IsDown())
			{
				c += ('a'-'A');
			}

			output += c;
		}
	}

	struct VKeyAsciiPair
	{
		VKEY vkey;
		char ascii;
		bool rangeStart;
	};

	const char BS_CHAR = 8;
	
	VKeyAsciiPair VKeyAsciiPairTable[] = 
	{
		{ VK_RETURN,		'\n',		false },
		{ VK_SPACE,			' ',		false },
		{ VK_TAB,			'\t',		false },
		{ VK_BACK,			BS_CHAR,	false },
		{ VK_OEM_COMMA,		',',		false },
		{ VK_OEM_PERIOD,	'.',		false },
		
		{ VK_NUMPAD0,		'0',		true },
		{ VK_NUMPAD9,		'9',		false },

		{ '0',				'0',		true },
		{ '9',				'9',		false },
	};

	for (int i = 0; i < ARRAY_SIZE(VKeyAsciiPairTable); ++i)
	{
		VKeyAsciiPair& currPair = VKeyAsciiPairTable[i];

		if (currPair.rangeStart)
		{
			++i; // Increment to range end value
			VKEY rangeEndVKey = VKeyAsciiPairTable[i].vkey;

			// Cycle through the range of vkeys and see if any of them are pressed
			for (VKEY currVKey = currPair.vkey; currVKey <= rangeEndVKey; ++currVKey)
			{
				if (kbMgr[currVKey].JustPressed())
				{
					output += currPair.ascii + static_cast<char>(currVKey - currPair.vkey);
				}
			}
		}
		else // Non-range key
		{
			if (kbMgr[currPair.vkey].JustPressed())
			{
				output += currPair.ascii;
			}
		}
	}

	return output;
}



