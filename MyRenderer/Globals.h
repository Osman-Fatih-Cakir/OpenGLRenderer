#pragma once

#include <string>

class Globals
{
public:

	static int WIDTH;
	static int HEIGHT;

	static void Log(std::string text);

	enum Projection_Type
	{
		PERSPECTIVE,
		ORTOGRAPHIC
	};
};
