#pragma once

#include <vector>

class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void keyboard(unsigned int key, int w, int h);

	bool key_press(unsigned int key);
	bool key_hold(unsigned int key);
	

private:
	

};