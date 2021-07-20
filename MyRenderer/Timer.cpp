
#include <Timer.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

// Constructor
Timer::Timer()
{
	old_time = (float)glutGet(GLUT_ELAPSED_TIME);
}

// Destructor
Timer::~Timer() {}

// Get delta time
float Timer::get_delta_time()
{
	cur_time = (float)glutGet(GLUT_ELAPSED_TIME);
	float delta = cur_time - old_time;
	old_time = cur_time;
	return delta;
}