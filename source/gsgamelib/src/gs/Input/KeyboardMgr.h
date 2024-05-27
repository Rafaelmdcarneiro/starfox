#ifndef _KEYBOARD_MGR_H_
#define _KEYBOARD_MGR_H_

#include "gs/Base/Singleton.h"
#include "gs/System/System.h"
#include <string>
//#include <map>
#include <array>
#include <cassert>

// Represents the state of one key. KeyboardMgr (below) manages
// an array of KeyStates.
class KeyState
{
public:
	enum Type { Up, Down, Pressed, Released };

	KeyState() : m_bLastDown(false), m_bCurrDown(false) { }

	// Returns true if key is currently down
	bool IsDown()	const			{ return m_bCurrDown; }

	// Returns true if key is currently up
	bool IsUp() const				{ return !m_bCurrDown; }

	// Returns the state enum value
	KeyState::Type GetState() const;

	// Returns true if the key was JUST pressed (true on first key down)
	bool JustPressed() const		{ return ( !m_bLastDown && m_bCurrDown ); }
	
	// Returns true if key was JUST released (true on first key up)
	bool JustReleased()	const		{ return ( m_bLastDown && !m_bCurrDown ); }

	bool operator==(const KeyState& rhs) const { return m_bCurrDown == rhs.m_bCurrDown && m_bLastDown == rhs.m_bLastDown; }
	bool operator!=(const KeyState& rhs) const { return !(*this==rhs); }

private: 
	// These private functions are called by KeyboardMgr
	friend class KeyboardMgr;

	void SetDown()
	{
		m_bLastDown = m_bCurrDown;
		m_bCurrDown = true;
	}

	void SetUp()
	{
		m_bLastDown = m_bCurrDown;
		m_bCurrDown = false;
	}

private:
	bool m_bLastDown;		// True if key was down last iteration
	bool m_bCurrDown;		// True if key is down this iteration
};


// Manages an array of KeyStates. The client queries the KeyboardMgr
// to get the state of specific keys.
class KeyboardMgr : public Singleton<KeyboardMgr>
{
private:
	friend class Singleton<KeyboardMgr>;
	KeyboardMgr() { }

public:
	// Must be called once per frame with elapsed frame time
	void Update(float32 frameTimeMS);

	const KeyState& GetKeyState(VKEY vkey) const
	{
		return m_keyStateMap[vkey];
	}

	// Same as GetKeyState() but more convenient
	const KeyState& operator[](VKEY vkey) const
	{
		return GetKeyState(vkey);
	}

	// Call to reset the internal state
	void SetAllKeysUp();

	// Returns a string with characters typed from the user. Does not
	// support all possible keys - just the most useful.
	std::string GetTypedInput();

private:
	// Updates key state for input key
	void UpdateKeyState(VKEY vkey, float32 frameTimeMS);

private:
	//typedef std::map<VKEY, KeyState> KeyStateMap;
	typedef std::array<KeyState, 256> KeyStateMap;
	KeyStateMap m_keyStateMap;
};


#endif // _KEYBOARD_MGR_H_
