#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	float get_delta_time()
	{
		cur_time = (float)glutGet(GLUT_ELAPSED_TIME);
		float delta = cur_time - old_time;
		old_time = cur_time;
		return delta;
	}

	float get_current_time()
	{
		return (float)glutGet(GLUT_ELAPSED_TIME);
	}

private:

	float old_time;
	float cur_time;
};
