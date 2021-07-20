#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

class Window
{
public:

	Window();
	~Window();

	void create_window(int w, int h);
	void destroy_window();

private:

	unsigned int win_id;

};
