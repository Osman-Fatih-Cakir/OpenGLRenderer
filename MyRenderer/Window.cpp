
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
}

// Destroys a window
void Window::destroy_window()
{
	glutDestroyWindow(win_id);
}