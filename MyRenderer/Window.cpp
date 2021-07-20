
#include <Window.h>

Window::Window()
{
}

Window::~Window()
{
}

// Creates a window
void Window::create_window(int w, int h)
{
	win_id = glutCreateWindow("Renderer Window");
	glutInitWindowSize(w, h);
	glutReshapeWindow(w, h);

	width = w;
	height = h;
}

// Destroys a window
void Window::destroy_window()
{
	glutDestroyWindow(win_id);
}

int Window::get_width()
{
	return width;
}

int Window::get_height()
{
	return height;
}
