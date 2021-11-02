#pragma once

#include <vector>

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
		if (std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end()) 
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
		if (std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end())
		{
			pressed_keys.erase(std::remove(pressed_keys.begin(), pressed_keys.end(), key),
				pressed_keys.end());
			return true;
		}
		else
		{
			return false;
		}
	}

	// TODO mouse
	// TODO add_mouse_event

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void add_key(Key key)
	{
		pressed_keys.push_back(key);
	}

	// WARNING: This method is called only by glut input functions
	// DO NOT CALL THIS FUNCTION!
	void remove_key(Key key)
	{
		pressed_keys.erase(std::remove(pressed_keys.begin(), pressed_keys.end(), key),
			pressed_keys.end());
	}

private:

	std::vector<Key> pressed_keys;
};

