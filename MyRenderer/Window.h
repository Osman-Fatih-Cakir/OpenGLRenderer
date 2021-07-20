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
	int get_width();
	int get_height();

private:

	unsigned int win_id;
	int width = 1024;
	int height = 1024;

};
