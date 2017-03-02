﻿#ifndef __INPUT_H__
#define __INPUT_H__
#include "common.h"
#include <Xinput.h>
BEGIN_NS

struct JoystickState
{
	bool valid;
	WORD buttons;
	BYTE leftTrigger;
	BYTE rightTrigger;
	SHORT thumbLX;
	SHORT thumbLY;
	SHORT thumbRX;
	SHORT thumbRY;
};

struct KeyboardState
{
	BYTE keystate[256];

	bool operator [](BYTE key)
	{
		return keystate[key] & 0x80;
	}
};

struct MouseState
{
	GMint deltaX;
	GMint deltaY;
};

struct IWindow;

#ifdef _WINDOWS
class Input_Windows
{
public:
	Input_Windows();
	~Input_Windows();

public:
	void initMouse(IWindow* window);

public:
	JoystickState getJoystickState();
	KeyboardState getKeyboardState();
	MouseState getMouseState();
private:
	bool m_mouseReady;
	IWindow* m_window;
};

typedef Input_Windows Input;
#endif

END_NS
#endif