#pragma once

#include <string>
#include <iostream>

#include "Globals.h"

int Globals::WIDTH = 1200;
int Globals::HEIGHT = 800;

// Print the texts to console
void Globals::Log(std::string text)
{
	std::cout << text << std::endl;
}

