#pragma once

#include <string>
#include <iostream>

class Globals
{
public:

	static const int WIDTH = 1200;
	static const int HEIGHT = 800;

	static void Log(std::string text);

	enum Projection_Type
	{
		PERSPECTIVE,
		ORTOGRAPHIC
	};
};
