#pragma once

#include <map>
//#include <vector>
#include <Timer.h>

#include <Keys.h>

// This class handles inputs
class Input
{
public:
	Input() = default;
	~Input() = default;

public:
	// Returns true if the key holds
	bool hold_key(Key key)
	{
		std::map<Key, bool>::iterator check = pressed_keys.find(key);
		if (check != pressed_keys.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns true if the key is pressed
	bool press_key(Key key)
	{
		std::map<Key, bool>::iterator check = pressed_keys.find(key);
		if (check != pressed_keys.end() && !check->second)
		{
			check->second = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns true if mouse has clicked
	bool click_mouse(MouseButton button)
	{
		std::map<MouseButton, bool>::iterator check = pressed_mouse_buttons.find(button);
		if (check != pressed_mouse_buttons.end() && !check->second)
		{
			check->second = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns true if the mouse hold
	bool hold_mouse(MouseButton button)
	{
		std::map<MouseButton, bool>::iterator check = pressed_mouse_buttons.find(button);
		if (check != pressed_mouse_buttons.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns mouse X position relative to window
	int mouseX()
	{
		return mouse_X;
	}

	// Returns mouse Y position relative to window
	int mouseY()
	{
		return mouse_Y;
	}

	// Returns delta mouse x position
	int mouse_deltaX()
	{
		int delta = mouse_X - prev_mouse_X;
		prev_mouse_X = mouse_X;
		return delta;
	}

	// Returns delta mouse y position
	int mouse_deltaY()
	{
		int delta = mouse_Y - prev_mouse_Y;
		prev_mouse_Y = mouse_Y;
		return delta;
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void add_key(Key key)
	{
		pressed_keys.insert(std::pair<Key, bool>(key, false));
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void remove_key(Key key)
	{
		pressed_keys.erase(key);
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void add_mouse_click(MouseButton button)
	{
		pressed_mouse_buttons.insert(std::pair<MouseButton, bool>(button, false));
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void remove_mouse_click(MouseButton button)
	{
		pressed_mouse_buttons.erase(button);
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void set_mouse(int x, int y)
	{
		prev_mouse_X = mouse_X;
		prev_mouse_Y = mouse_Y;
		mouse_X = x;
		mouse_Y = y;
	}

private:

	std::map<Key, bool> pressed_keys;
	std::map<MouseButton, bool> pressed_mouse_buttons;

	int mouse_X = 0;
	int mouse_Y = 0;
	int prev_mouse_X = 0;
	int prev_mouse_Y = 0;
};

